#pragma once

#include <string>
#include <winlib/raii.h>
#include <windows.h>
#include <wincrypt.h>


namespace drjuke::scanlib
{
    enum class CertificateInfoType
    {
        kPublisherInfo,
        kMoreInfo
    };

    struct CertificateInfo
    {
        std::wstring m_program_name;
        std::wstring m_publisher_link;
        std::wstring m_more_info_link;
        std::string  m_serial_number;
        SYSTEMTIME   m_timestamp;
    };

    struct CertificateInfoBuilder
    {
    private:
        std::wstring        m_target_filename;

        DWORD               m_encoding_type;
        DWORD               m_content_type;
        DWORD               m_format_type;
        winlib::UniqueBlob  p_signer_info;
        winlib::UniqueBlob  p_counter_signer_info;
        CERT_INFO           CertInfo;     
        SYSTEMTIME          m_date_of_timestamp;

        CertificateInfo m_details;

    private:
        void initializeCryptoObjects();
        void getProgramAndPublisherInfo();
        void getTimestamp();
        void getSignerInfo();

        // Вспомогательные методы
        static CRYPT_ATTRIBUTE getNecessaryAttribute(CRYPT_ATTRIBUTES attrs, const std::string &object_id);
        static std::wstring getCertificateInfo(PSPC_SP_OPUS_INFO opus_info, CertificateInfoType cert_info);

    public:
        explicit CertificateInfoBuilder(const wchar_t *filename);

        [[nodiscard]] CertificateInfo get() const;
    };
}
