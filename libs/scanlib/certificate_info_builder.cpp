#include "certificate_info_builder.h"

#include <iostream>
#include <wintrust.h>

#include <winlib/winlib.h> 

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
#define NO_INFO L"no information"


namespace drjuke::scanlib
{
    using winlib::WindowsException;
    using winlib::UniqueBlob;
    using winlib::AllocateBlob;

    void CertificateInfoBuilder::initializeCryptoObjects()
    {
        
        DWORD      signer_info_size   = 0;; // Размер структуры информации о подписавшем
        HCRYPTMSG  crypt_message      = nullptr ;
        HCERTSTORE certificate_store  = nullptr;
        
        // Получаем информацию о криптографическом объекте
        auto status = CryptQueryObject
        (
            CERT_QUERY_OBJECT_FILE,                     // Тип объекта (файл или блоб)
            m_target_filename.c_str(),                  // Путь к файлу
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, // Что внутри, ожидаем подписанный файл
            CERT_QUERY_FORMAT_FLAG_BINARY,              // В каком формате находится объект
            0,                                          // Всегда 0
            &m_encoding_type,                           // Как закодировано
            &m_content_type,
            &m_format_type,
            &certificate_store,                         // Здесь будет хендл на хранилище сертификатов
            &crypt_message,                             // Сюда записывается открытое сообщение
            nullptr
        );

        if (!status)
        {
            throw WindowsException("CryptQueryObject fail");
        }

        // Получаем размер структуры, которую вернет функция
        status = CryptMsgGetParam
        (
            crypt_message, 
            CMSG_SIGNER_INFO_PARAM, 
            0, 
            nullptr, 
            &signer_info_size
        );

        if (!status)
        {
            throw WindowsException("CryptMsgGetParam fail");
        }

        // Выделяем память под SIGNER_INFO
        p_signer_info = UniqueBlob(AllocateBlob(signer_info_size));

        // Получаем информацию о подписавшем
        status = CryptMsgGetParam
        (
            crypt_message, 
            CMSG_SIGNER_INFO_PARAM, 
            0, 
            p_signer_info.get(), 
            &signer_info_size
        );

        if (!status)
        {
            throw WindowsException("CryptMsgGetParam fail");
        }
    }

    void CertificateInfoBuilder::getProgramAndPublisherInfo()
    {  
        DWORD opus_info_size;

        auto auth_attrs = static_cast<PCMSG_SIGNER_INFO>(p_signer_info.get())->AuthAttrs;
        auto attribute = getNecessaryAttribute(auth_attrs, SPC_SP_OPUS_INFO_OBJID);

        // Get Size of SPC_SP_OPUS_INFO structure.
        BOOL status = CryptDecodeObject
        (
            ENCODING,
            SPC_SP_OPUS_INFO_OBJID,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            nullptr,
            &opus_info_size
        );

        if (!status)
        {
            throw WindowsException("CryptDecodeObject failed");
        }

        // Allocate memory for SPC_SP_OPUS_INFO structure.
        auto opus_info     = UniqueBlob(AllocateBlob(opus_info_size));
        auto opus_info_ptr = static_cast<PSPC_SP_OPUS_INFO>(opus_info.get());

        if (!opus_info)
        {
            throw std::runtime_error("Unable to allocate memory for Publisher Info");
        }

        // Decode and get SPC_SP_OPUS_INFO structure.
        status = CryptDecodeObject
        (
            ENCODING,
            SPC_SP_OPUS_INFO_OBJID,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            opus_info_ptr,
            &opus_info_size
        );

        if (!status)
        {
            throw WindowsException("CryptDecodeObject failed");
        }

        // Fill in Program Name if present.
        m_details.m_program_name = 
            opus_info_ptr->pwszProgramName ?
            opus_info_ptr->pwszProgramName :
            NO_INFO;

        // Fill in Publisher Information if present.
        m_details.m_publisher_link = 
            opus_info_ptr->pPublisherInfo ?
            getCertificateInfo(opus_info_ptr, CertificateInfoType::kPublisherInfo) : 
            NO_INFO;

        // Fill in More Info if present.
        m_details.m_more_info_link =
            opus_info_ptr->pMoreInfo ?
            getCertificateInfo(opus_info_ptr, CertificateInfoType::kMoreInfo) :
            NO_INFO;
    }

    void CertificateInfoBuilder::getTimestamp()
    {
        FILETIME        local_filetime;
        FILETIME        filetime;
        DWORD           info_size = sizeof(FILETIME);
        CRYPT_ATTRIBUTE attribute;

        // TODO: Избавиться от копипасты
        auto auth_attrs = static_cast<PCMSG_SIGNER_INFO>(p_signer_info.get())->AuthAttrs;

        try
        {
            attribute = getNecessaryAttribute(auth_attrs, szOID_RSA_signingTime);
        }
        catch (const std::runtime_error&)
        {
            // TODO: поставить not found в отчете
            std::cout << "getTimestamp - no attribute" << std::endl;
            return;
        }

        BOOL status = CryptDecodeObject
        (
            ENCODING,
            szOID_RSA_signingTime,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            static_cast<PVOID>(&filetime),
            &info_size
        );

        if (!status)
        {
            throw WindowsException("CryptDecodeObject failed");
        }

        // Convert to local time.
        FileTimeToLocalFileTime(&filetime, &local_filetime);
        FileTimeToSystemTime(&local_filetime, &m_details.m_timestamp);
    } 

    void CertificateInfoBuilder::getSignerInfo()
    {    
        DWORD dw_size = 0;
        CRYPT_ATTRIBUTE attribute;

        auto auth_attrs = static_cast<PCMSG_SIGNER_INFO>(p_signer_info.get())->AuthAttrs;

        try
        {
            attribute = getNecessaryAttribute(auth_attrs, szOID_RSA_counterSign);
        }
        catch (const std::runtime_error&)
        {
            // TODO: поставить not found в отчете
            std::cout << "getSignerInfo - no attribute" << std::endl;
            return;
        }

        // Get size of CMSG_SIGNER_INFO structure.
        auto status = CryptDecodeObject
        (
            ENCODING,
            PKCS7_SIGNER_INFO,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            nullptr,
            &dw_size
        );

        if (!status)
        {
            throw WindowsException("CryptDecodeObject failed");
        }

        // Allocate memory for CMSG_SIGNER_INFO.

        p_counter_signer_info = UniqueBlob(AllocateBlob(dw_size));
        
        if (!p_counter_signer_info)
        {
            throw std::runtime_error("Unable to allocate memory for timestamp info.");
        }

        // Decode and get CMSG_SIGNER_INFO structure
        // for timestamp certificate.
        status = CryptDecodeObject
        (
            ENCODING,
            PKCS7_SIGNER_INFO,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            p_counter_signer_info.get(),
            &dw_size
        );

        if (!status)
        {
            throw WindowsException("CryptDecodeObject failed");
        }
    }

    CRYPT_ATTRIBUTE CertificateInfoBuilder::getNecessaryAttribute(CRYPT_ATTRIBUTES attrs, const std::string &object_id)
    {
        for (DWORD i = 0; i < attrs.cAttr; i++)
        {
            auto obj_id = attrs.rgAttr[i].pszObjId;

            if (std::string(obj_id) == object_id)
            {
                return attrs.rgAttr[i];
            }
        }

        throw std::runtime_error("Specified attribute not found");
    }

    std::wstring CertificateInfoBuilder::getCertificateInfo(PSPC_SP_OPUS_INFO opus_info, CertificateInfoType cert_info)
    {
        SPC_LINK *link;

        switch (cert_info)
        {
        case CertificateInfoType::kMoreInfo      : link = opus_info->pMoreInfo;      break;
        case CertificateInfoType::kPublisherInfo : link = opus_info->pPublisherInfo; break;
        default                                  : link = opus_info->pPublisherInfo; break;
        }

        switch (link->dwLinkChoice)
        {
        case SPC_URL_LINK_CHOICE  :  return link->pwszUrl;
        case SPC_FILE_LINK_CHOICE :  return link->pwszFile;
        default                   :  return L"";
        }
    }

    // !! ИМЯ ФАЙЛА !!
    CertificateInfoBuilder::CertificateInfoBuilder(const wchar_t *filename)
        : m_target_filename(filename)
    {
        initializeCryptoObjects();
        getProgramAndPublisherInfo();
        getTimestamp();
        getSignerInfo();
    }

    CertificateInfo CertificateInfoBuilder::get() const
    {
        return m_details;
    }
}
