#include "certificate_analyzer.h"
#include "certificate_info_builder.h"

#pragma comment( lib, "wintrust.lib" )
#pragma comment( lib, "crypt32.lib"  )

#pragma warning( disable : 26812 ) // Unscoped enum

#include <type_traits>

#include <common/win_raii.h>
#include <winlib/windows_exception.h>

namespace 
{
    constexpr std::string_view kFileSigned          { "The file is signed"           };
    constexpr std::string_view kFileNotSigned       { "The file is not signed"       };
    constexpr std::string_view kSignatureNotTrusted { "The signature is not trusted" };
    constexpr std::string_view kFileCorrupted       { "The file is corrupted"        };
    constexpr std::string_view kRootUntrusted       { "The Root CA is untrusted"     };
    constexpr std::string_view kCertificateExpired  { "The certificate is expired"   };
    constexpr std::string_view kCertificateRevoked  { "The certificate is revoked"   };
}

namespace drjuke::scanlib
{
    namespace 
    {
        struct SignatureStatus { enum Type
        {
            kSuccess                      = 0,
            kActionUnknown                = 0x800b0002,                   // Trust provider does not support the specified action
            kSubjectNotTrusted            = 0x800b0004,                   // Subject failed the specified verification action
            kSubjectExplicitlyDistrusted  = 0x800B0111,                   // Signer's certificate is in the Untrusted Publishers store
            kProviderUnknown              = TRUST_E_PROVIDER_UNKNOWN,     // Trust provider is not recognized on this system
            kSubjectFormUnknown           = TRUST_E_SUBJECT_FORM_UNKNOWN, // Trust provider does not support the form specified for the subject
            kFileNotSigned                = TRUST_E_NOSIGNATURE,          // Файл не подписан
            kSignatureOrFileCorrupt       = TRUST_E_BAD_DIGEST,           // Файл поврежден
            kSubjectCertExpired           = CERT_E_EXPIRED,               // Время действия сертификата закончилось
            kSubjectCertificateRevoked    = CERT_E_REVOKED,               // Сертификат отозван
            kUntrustedRoot                = CERT_E_UNTRUSTEDROOT          // Корневой удостоверяющий центр является недоверенным
        }; };

        std::map<std::underlying_type_t<SignatureStatus::Type>, std::string_view> g_FormattedStatuses 
        {
            { SignatureStatus::kSuccess                    , kFileSigned           }, 
            { SignatureStatus::kProviderUnknown            , kFileNotSigned        },
            { SignatureStatus::kActionUnknown              , kFileNotSigned        },
            { SignatureStatus::kSubjectFormUnknown         , kFileNotSigned        },
            { SignatureStatus::kSubjectNotTrusted          , kSignatureNotTrusted  },
            { SignatureStatus::kSubjectExplicitlyDistrusted, kSignatureNotTrusted  },
            { SignatureStatus::kFileNotSigned              , kFileNotSigned        },
            { SignatureStatus::kSignatureOrFileCorrupt     , kFileCorrupted        },
            { SignatureStatus::kSubjectCertExpired         , kCertificateExpired   },
            { SignatureStatus::kSubjectCertificateRevoked  , kCertificateRevoked   },
            { SignatureStatus::kUntrustedRoot              , kRootUntrusted        }
        };
    }

    Json SignatureReport::makeJson()
    {
        return m_report;
    }

    void DigitalSignatureAnalyzer::constructWinTrustFileInfo(const wchar_t *filename)
    {
        memset(&m_file_info, 0, sizeof(m_file_info));

        m_file_info.cbStruct       = sizeof(WINTRUST_FILE_INFO);
        m_file_info.pcwszFilePath  = filename;
        m_file_info.hFile          = nullptr;
        m_file_info.pgKnownSubject = nullptr;
    }

    // TODO: unique_ptr
    void DigitalSignatureAnalyzer::constructWinTrustData()
    {
        // TODO: перевести
        /*
        WVTPolicyGUID определяет политике для применения к файлу
        WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

        1) The certificate used to sign the file chains up to a root 
        certificate located in the trusted root certificate store. This 
        implies that the identity of the publisher has been verified by 
        a certification authority.

        2) In cases where user interface is displayed (which this example
        does not do), WinVerifyTrust will check for whether the  
        end entity certificate is stored in the trusted publisher store,  
        implying that the user trusts content from this publisher.

        3) The end entity certificate has sufficient permission to sign 
        code, as indicated by the presence of a code signing EKU or no 
        EKU.
        */

        memset(&m_win_trust_data, 0, sizeof(m_win_trust_data)); // Инициализировать все в 0.

        m_win_trust_data.cbStruct             = sizeof(m_win_trust_data); // Размер структуры
        m_win_trust_data.pPolicyCallbackData  = nullptr;                  // Use default code signing EKU.
        m_win_trust_data.pSIPClientData       = nullptr;                  // No data to pass to SIP.
        m_win_trust_data.dwUIChoice           = WTD_UI_NONE;              // Disable WVT UI.
        m_win_trust_data.fdwRevocationChecks  = WTD_REVOKE_NONE;          // No revocation checking.
        m_win_trust_data.dwUnionChoice        = WTD_CHOICE_FILE;          // Verify an embedded signature on a file.
        m_win_trust_data.dwStateAction        = WTD_STATEACTION_VERIFY;   // Verify action.
        m_win_trust_data.hWVTStateData        = nullptr;                  // Verification sets this value.
        m_win_trust_data.pwszURLReference     = nullptr;                  // Не используется
        m_win_trust_data.dwUIContext          = 0;                        // Зануляем контекст UI, так как не используем UI
        m_win_trust_data.pFile                = &m_file_info;             // Непосредственно данные файла
    }

    void DigitalSignatureAnalyzer::destroyWinTrustData()
    {
        m_win_trust_data.dwStateAction = WTD_STATEACTION_CLOSE;

        WinVerifyTrust
        (
            nullptr,
            &m_wvt_policy_guid,
            &m_win_trust_data
        );
    }

    BaseReportPtr DigitalSignatureAnalyzer::getReport(const Path &path)
    {
        // Получаем путь в приемлемом для WinAPI представлении
        const auto str_path     = path.generic_wstring();
        const auto filename_ptr = str_path.c_str();

        std::string resolution;

        constructWinTrustFileInfo(filename_ptr);
        constructWinTrustData();

        // Проверяем подпись
        auto status = WinVerifyTrust
        (
            nullptr,
            &m_wvt_policy_guid,
            &m_win_trust_data
        );

        destroyWinTrustData();

        // TODO: Собственные исключения
        // TODO: Заменить shared на unique

        try
        {
            auto str_status = g_FormattedStatuses[status];

            if (str_status != kFileNotSigned)
            {
                auto details = CertificateInfoBuilder(filename_ptr).get();
                return std::make_shared<SignatureReport>(str_status.data(), details);
            }

            // TODO: Залогировать
            return std::make_shared<SignatureReport>(str_status.data());
        }
        catch (const std::system_error&)
        {
            return std::make_shared<SignatureReport>(g_FormattedStatuses[status].data());
        }
        catch (const std::out_of_range&)
        {
            return std::make_shared<SignatureReport>("Unknown");
        }

    }

    void DigitalSignatureAnalyzer::loadResources()
    {
        // Не делать ничего, так как для проверки 
        // ЭЦП используется исключительно WinAPI
    }

    std::string DigitalSignatureAnalyzer::getName()
    {
        return "Signature analyzer";
    }

    void SignatureReport::initializeJson()
    {
        // TODO: задефайнить строки 
        m_report["resolution"]              = "UNKNOWN";
        m_report["signer"]                  = "UNKNOWN";
        m_report["application"]             = "UNKNOWN";
        m_report["url"]                     = "UNKNOWN";
        m_report["more_info"]               = "UNKNOWN";
        m_report["datetime"]["year"]        = -1;
        m_report["datetime"]["month"]       = -1;
        m_report["datetime"]["day"]         = -1;
        m_report["datetime"]["day of week"] = -1;
        m_report["datetime"]["hour"]        = -1;
        m_report["datetime"]["minute"]      = -1;
        m_report["datetime"]["second"]      = -1;
        m_report["datetime"]["millisecond"] = -1;
    }

    void SignatureReport::fillJsonWithStatus()
    {
        m_report["resolution"] = m_status;
    }

    void SignatureReport::fillJsonWithDetails()
    {
        m_report["signer"]                  = m_details.m_publisher_link;
        m_report["application"]             = m_details.m_program_name;
        m_report["url"]                     = m_details.m_publisher_link;
        m_report["more_info"]               = m_details.m_more_info_link;
        m_report["datetime"]["year"]        = m_details.m_timestamp.wYear;
        m_report["datetime"]["month"]       = m_details.m_timestamp.wMonth;
        m_report["datetime"]["day"]         = m_details.m_timestamp.wDay;
        m_report["datetime"]["day of week"] = m_details.m_timestamp.wDayOfWeek;
        m_report["datetime"]["hour"]        = m_details.m_timestamp.wHour;
        m_report["datetime"]["minute"]      = m_details.m_timestamp.wMinute;
        m_report["datetime"]["second"]      = m_details.m_timestamp.wSecond;
        m_report["datetime"]["millisecond"] = m_details.m_timestamp.wMilliseconds;
    }
}
