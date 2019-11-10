#pragma once

#include <string>
#include <common/aliases.h>

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>

namespace drjuke::scansvc
{
    typedef struct 
    {
        std::wstring lpszProgramName;
        std::wstring lpszPublisherLink;
        std::wstring lpszMoreInfoLink;
    } 
    SPROG_PUBLISHERINFO, 
    *PSPROG_PUBLISHERINFO;

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

        std::string m_timestamp;
        std::string m_serial_number;
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
        SPROG_PUBLISHERINFO ProgPubInfo;
        SYSTEMTIME          m_date_of_timestamp;
        SPROG_PUBLISHERINFO m_publisher_info;

    private:
        void initialize();
        void getProgramAndPublisherInfo();
        void getDateOfTimestamps();
        void getSignerTimestamps();

        // Вспомогательные методы
        CRYPT_ATTRIBUTE getNecessaryAttribute(PCMSG_SIGNER_INFO info, const std::string &object_id);
        static std::wstring getCertificateInfo(PSPC_SP_OPUS_INFO opus_info, CertificateInfo cert_info);

    public:
        explicit CertificateViewer(const wchar_t *filename);

        [[nodiscard]] CertificateDetails getDetails();
    };
}
