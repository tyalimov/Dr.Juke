#include "signature_analyzer.h"
#include "cert_viewer.h"

#pragma comment( lib, "wintrust.lib" )
#pragma comment( lib, "crypt32.lib"  )

#include <common/win_raii.h>

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

namespace drjuke::scansvc
{


    SignatureReport::SignatureReport()
    {

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

    void SignatureAnalyzer::processSigned(const wchar_t *filename)
    {
        /*
        Signed file:
        - Hash that represents the subject is trusted.

        - Trusted publisher without any verification errors.

        - UI was disabled in dwUIChoice. No publisher or 
        time stamp chain errors.

        - UI was enabled in dwUIChoice and the user clicked 
        "Yes" when asked to install and run the signed 
        subject.
        */
        wprintf_s(L"The file \"%s\" is signed and the signature "
            L"was verified.\n",
            filename);
    }

    void SignatureAnalyzer::processNotSigned(const wchar_t *filename)
    {
        auto last_error = GetLastError();

        if 
        (
            last_error == TRUST_E_NOSIGNATURE          ||
            last_error == TRUST_E_SUBJECT_FORM_UNKNOWN ||
            last_error == TRUST_E_PROVIDER_UNKNOWN
        ) 
        {
            // Файл не подписан
            wprintf_s(L"The file \"%s\" is not signed.\n", filename);
        } 
        else 
        {
            // The signature was not valid or there was an error 
            // opening the file.
            wprintf_s(L"An unknown error occurred trying to verify the signature of the \"%s\" file.\n", filename);
        }
    }

    void SignatureAnalyzer::processDisallowded(const wchar_t *filename)
    {
        wprintf_s(L"The signature is present, but specifically "
            L"disallowed.\n");

        filename;
    }

    void SignatureAnalyzer::processNonTrusted(const wchar_t *filename)
    {
        filename;
    }

    void SignatureAnalyzer::processError(LONG status)
    {
        wprintf_s(L"Error is: 0x%x.\n", status);
    }

    void SignatureAnalyzer::getSertificateDetails(const wchar_t *filename)
    {
        auto details = CertificateViewer(filename).getDetails();
    }

    IReportPtr SignatureAnalyzer::getReport(const Path &path)
    {
        // Получаем путь в приемлемом для WinAPI представлении
        const auto str_path     = path.generic_wstring();
        const auto filename_ptr = str_path.c_str();

        constructWinTrustFileInfo(filename_ptr);
        constructWinTrustData();

        // Проверяем подпись
        auto status = WinVerifyTrust
        (
            nullptr,
            &m_wvt_policy_guid,
            &m_win_trust_data
        );

        switch (status) 
        {
            case ERROR_SUCCESS:               processSigned(filename_ptr);      break; // Файл подписан валидной подписью
            case TRUST_E_NOSIGNATURE:         processNotSigned(filename_ptr);   break; // Файл не подписан, либо подпись невалидна
            case TRUST_E_EXPLICIT_DISTRUST:   processDisallowded(filename_ptr); break; // Подпись не разрешена политикой безопасности
            case TRUST_E_SUBJECT_NOT_TRUSTED: processNonTrusted(filename_ptr);  break; // Подпись не является доверенной
            case CRYPT_E_SECURITY_SETTINGS:   processDisallowded(filename_ptr); break; // Подпись не разрешена политикой безопасности
            default:                          processError(status);             break; // Произошла ошибка при проверке
        }

        destroyWinTrustData();

        return std::make_shared<SignatureReport>();
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
}