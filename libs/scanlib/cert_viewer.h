#pragma once

#include <string>
#include <common/aliases.h>
#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>

namespace drjuke::scanlib
{
    enum class CertificateInfo
    {
        kPublisherInfo,
        kMoreInfo
    };

    struct CertificateDetails
    {
        std::wstring m_program_name;
        std::wstring m_publisher_link;
        std::wstring m_more_info_link;
        std::string  m_serial_number;
        SYSTEMTIME   m_timestamp{0,0,0,0,0,0,0,0};
    };

    struct CertificateViewer
    {
    private:
        std::wstring        m_target_filename;
        HCERTSTORE          hStore               = nullptr;
        HCRYPTMSG           hMsg                 = nullptr; 
        PCCERT_CONTEXT      pCertContext         = nullptr;
        BOOL                fResult;   
        DWORD               dwEncoding;
        DWORD               dwContentType;
        DWORD               dwFormatType;
        PCMSG_SIGNER_INFO   pSignerInfo          = nullptr;
        PCMSG_SIGNER_INFO   pCounterSignerInfo   = nullptr;
        DWORD               dwSignerInfo;
        CERT_INFO           CertInfo;     
        SYSTEMTIME          m_date_of_timestamp;

        CertificateDetails m_details;

    private:
        void initialize();
        void getProgramAndPublisherInfo();
        void getTimestamp();
        void getSignerInfo();

        // Вспомогательные методы
        CRYPT_ATTRIBUTE getNecessaryAttribute(PCMSG_SIGNER_INFO info, const std::string &object_id);
        static std::wstring getCertificateInfo(PSPC_SP_OPUS_INFO opus_info, CertificateInfo cert_info);

    public:
        explicit CertificateViewer(const wchar_t *filename);

        [[nodiscard]] CertificateDetails getDetails();
    };
}
