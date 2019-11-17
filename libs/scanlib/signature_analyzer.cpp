#include "signature_analyzer.h"
#include "cert_viewer.h"

#pragma comment( lib, "wintrust.lib" )
#pragma comment( lib, "crypt32.lib"  )

#pragma warning( disable : 26812 ) // Unscoped enum

#include <common/win_raii.h>

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

namespace drjuke::scanlib
{
    namespace 
    {
        struct SignatureStatus { enum 
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

        std::map<uint32_t, std::string> g_FormattedStatuses = 
        {
            { SignatureStatus::kSuccess                    , "The file is signed"           },
            { SignatureStatus::kProviderUnknown            , "The file is not signed"       },
            { SignatureStatus::kActionUnknown              , "The file is not signed"       },
            { SignatureStatus::kSubjectFormUnknown         , "The file is not signed"       },
            { SignatureStatus::kSubjectNotTrusted          , "The signature is not trusted" },
            { SignatureStatus::kSubjectExplicitlyDistrusted, "The signature is not trusted" },
            { SignatureStatus::kFileNotSigned              , "The file is not signed"       },
            { SignatureStatus::kSignatureOrFileCorrupt     , "The file is corrupted"        },
            { SignatureStatus::kSubjectCertExpired         , "The certificate is expired"   },
            { SignatureStatus::kSubjectCertificateRevoked  , "The certificate is revoked"   },
            { SignatureStatus::kUntrustedRoot              , "The Root CA is untrusted"     }
        };

        std::array<int32_t, 6> g_RequiredDetaledInfo =
        {
            SignatureStatus::kSuccess,
            SignatureStatus::kSubjectNotTrusted,
            SignatureStatus::kSubjectExplicitlyDistrusted,
            SignatureStatus::kSubjectCertExpired,
            SignatureStatus::kSubjectCertificateRevoked,
            SignatureStatus::kUntrustedRoot
        };
    }

    Json SignatureReport::toJson()
    {
        return Json();
    }

    void SignatureAnalyzer::constructWinTrustFileInfo(const wchar_t *filename)
    {
        //WINTRUST_FILE_INFO info;

        memset(&m_file_info, 0, sizeof(m_file_info));

        m_file_info.cbStruct       = sizeof(WINTRUST_FILE_INFO);
        m_file_info.pcwszFilePath  = filename;
        m_file_info.hFile          = nullptr;
        m_file_info.pgKnownSubject = nullptr;
    }

    // TODO: unique_ptr
    void SignatureAnalyzer::constructWinTrustData()
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

        memset(&m_win_trust_data, 0, sizeof(m_win_trust_data)); // Default all fields to 0.

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

    void SignatureAnalyzer::destroyWinTrustData()
    {
        m_win_trust_data.dwStateAction = WTD_STATEACTION_CLOSE;

        WinVerifyTrust
        (
            nullptr,
            &m_wvt_policy_guid,
            &m_win_trust_data
        );
    }

    IReportPtr SignatureAnalyzer::getReport(const Path &path)
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
        try
        {
            auto str_status = g_FormattedStatuses[status];
            auto details    = CertificateViewer(filename_ptr).getDetails();

            // TODO: Залогировать
            return std::make_shared<SignatureReport>(str_status, details);
        }
        catch (const std::system_error &ex)
        {
            UNREFERENCED_PARAMETER(ex);
            // TODO: Залогировать
            return std::make_shared<SignatureReport>(g_FormattedStatuses[status]);
        }
        catch (const std::out_of_range &ex)
        {
            UNREFERENCED_PARAMETER(ex);
            // TODO: Залогировать
            return std::make_shared<SignatureReport>("Unknown");
        }
    }

    void SignatureAnalyzer::loadResources()
    {
        // Не делать ничего, так как для проверки 
        // ЭЦП используется исключительно WinAPI
    }

    std::string SignatureAnalyzer::getName()
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
        m_report["datetime"]["year"]        = -1;
        m_report["datetime"]["month"]       = -1;
        m_report["datetime"]["day"]         = -1;
        m_report["datetime"]["day of week"] = -1;
        m_report["datetime"]["hour"]        = -1;
        m_report["datetime"]["minute"]      = -1;
        m_report["datetime"]["second"]      = -1;
        m_report["datetime"]["millisecond"] = -1;
    }
}
