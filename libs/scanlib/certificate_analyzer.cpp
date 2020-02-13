#include "certificate_analyzer.h"

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
            { SignatureStatus::kSuccess                    , kFileSigned     }, 
            { SignatureStatus::kProviderUnknown            , kFileSigned     },
            { SignatureStatus::kActionUnknown              , kFileSigned     },
            { SignatureStatus::kSubjectFormUnknown         , kFileSigned     },
            { SignatureStatus::kSubjectNotTrusted          , kFileSigned     },
            { SignatureStatus::kSubjectExplicitlyDistrusted, kFileSigned     },
            { SignatureStatus::kFileNotSigned              , kFileNotSigned  },
            { SignatureStatus::kSignatureOrFileCorrupt     , kFileCorrupted  },
            { SignatureStatus::kSubjectCertExpired         , kFileSigned     },
            { SignatureStatus::kSubjectCertificateRevoked  , kFileSigned     },
            { SignatureStatus::kUntrustedRoot              , kFileSigned     }
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

    void DigitalSignatureAnalyzer::constructWinTrustData()
    {
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

        try
        {
            auto str_status = g_FormattedStatuses[status];
            return std::make_shared<SignatureReport>(str_status.data());
        }
        catch (const std::exception& /*ex*/)
        {

        }

        return std::make_shared<SignatureReport>(kFileNotSigned.data()); 

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
    }

    void SignatureReport::fillJsonWithStatus()
    {
        m_report["infected"]  = m_status == kFileSigned ? false : true;
        m_report["name"]      = "Signature";
    }
}
