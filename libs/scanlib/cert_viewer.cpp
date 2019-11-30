#include "cert_viewer.h"

#include <iostream>
#include <wintrust.h>

#include <memory>

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
#define NO_INFO L"no information"

namespace drjuke::scanlib
{
    void CertificateViewer::initialize()
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
            // TODO: логировать
            std::cout << "CryptQueryObject failed with" << GetLastError() << std::endl;;
            return;
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
            // TODO: логировать
            std::cout << "CryptMsgGetParam failed with " << GetLastError() << std::endl;
            return;
        }

        // Выделяем память под SIGNER_INFO
        pSignerInfo = static_cast<PCMSG_SIGNER_INFO>(malloc(signer_info_size)); // TODO: unique_ptr

        if (!pSignerInfo)
        {
            // TODO: логировать
            std::cout << "Unable to allocate memory for Signer Info" << std::endl;
            return;
        }

        // Получаем информацию о подписавшем
        status = CryptMsgGetParam
        (
            crypt_message, 
            CMSG_SIGNER_INFO_PARAM, 
            0, 
            static_cast<PVOID>(pSignerInfo), 
            &signer_info_size
        );

        if (!status)
        {
            // TODO: логировать
            std::cout << "CryptMsgGetParam failed with " << GetLastError() << std::endl;
            return;
        }
    }

    void CertificateViewer::getProgramAndPublisherInfo()
    {
        PSPC_SP_OPUS_INFO opus_info = nullptr;  
        DWORD dwData{0};
        auto attribute = getNecessaryAttribute(pSignerInfo->AuthAttrs, SPC_SP_OPUS_INFO_OBJID);

        // Get Size of SPC_SP_OPUS_INFO structure.
        BOOL status = CryptDecodeObject
        (
            ENCODING,
            SPC_SP_OPUS_INFO_OBJID,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            nullptr,
            &dwData
        );

        if (!status)
        {
            std::cout << "CryptDecodeObject failed with " << GetLastError() << std::endl;
            return;
        }

        // Allocate memory for SPC_SP_OPUS_INFO structure.
        opus_info = static_cast<PSPC_SP_OPUS_INFO>(LocalAlloc(LPTR, dwData));

        if (!opus_info)
        {
            std::cout << "Unable to allocate memory for Publisher Info\n";
            return;
        }

        // Decode and get SPC_SP_OPUS_INFO structure.
        status = CryptDecodeObject
        (
            ENCODING,
            SPC_SP_OPUS_INFO_OBJID,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            opus_info,
            &dwData
        );

        if (!status)
        {
            std::cout << "CryptDecodeObject failed with " << GetLastError() << std::endl;
            return;
        }


        // Fill in Program Name if present.
        m_details.m_program_name = 
            opus_info->pwszProgramName ?
            opus_info->pwszProgramName :
            NO_INFO;

        // Fill in Publisher Information if present.
        m_details.m_publisher_link = 
            opus_info->pPublisherInfo ?
            getCertificateInfo(opus_info, CertificateInfo::kPublisherInfo) : 
            NO_INFO;

        // Fill in More Info if present.
        m_details.m_more_info_link =
            opus_info->pMoreInfo ?
            getCertificateInfo(opus_info, CertificateInfo::kMoreInfo) :
            NO_INFO;
    }

    void CertificateViewer::getTimestamp()
    {
        FILETIME        local_filetime;
        FILETIME        filetime;
        DWORD           info_size = sizeof(FILETIME);
        CRYPT_ATTRIBUTE attribute;

        try
        {
            attribute = getNecessaryAttribute(pSignerInfo->AuthAttrs, szOID_RSA_signingTime);
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
            std::cout << "CryptDecodeObject failed with: " << GetLastError() << std::endl;
            return;
        }

        // Convert to local time.
        FileTimeToLocalFileTime(&filetime, &local_filetime);
        FileTimeToSystemTime(&local_filetime, &m_details.m_timestamp);
    } 

    void CertificateViewer::getSignerInfo()
    {    
        DWORD dw_size = 0;
        pCounterSignerInfo = nullptr;
        CRYPT_ATTRIBUTE attribute;

        try
        {
            attribute = getNecessaryAttribute(pSignerInfo->AuthAttrs, szOID_RSA_counterSign);
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
            std::cout << "CryptDecodeObject failed with " << GetLastError() << std::endl;
            return;
        }

        // Allocate memory for CMSG_SIGNER_INFO.
        pCounterSignerInfo = static_cast<PCMSG_SIGNER_INFO>(LocalAlloc(LPTR, dw_size));

        if (!pCounterSignerInfo)
        {
            std::cout << "Unable to allocate memory for timestamp info." << GetLastError() << std::endl;
            return;
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
            static_cast<PVOID>(pCounterSignerInfo),
            &dw_size
        );

        if (!status)
        {
            std::cout << "CryptDecodeObject failed with " << GetLastError() << std::endl;
            return;
        }

        //pCounterSignerInfo->Issuer
    }

    CRYPT_ATTRIBUTE CertificateViewer::getNecessaryAttribute(CRYPT_ATTRIBUTES attrs, const std::string &object_id)
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
        // TODO: Выбросить исключение, если ничего не нашли.
    }

    std::wstring CertificateViewer::getCertificateInfo(PSPC_SP_OPUS_INFO opus_info, CertificateInfo cert_info)
    {
        SPC_LINK *link;

        switch (cert_info)
        {
        case CertificateInfo::kMoreInfo      : link = opus_info->pMoreInfo;      break;
        case CertificateInfo::kPublisherInfo : link = opus_info->pPublisherInfo; break;
        default                              : link = opus_info->pPublisherInfo; break;
        }

        switch (link->dwLinkChoice)
        {
        case SPC_URL_LINK_CHOICE  :  return link->pwszUrl;
        case SPC_FILE_LINK_CHOICE :  return link->pwszFile;
        default                   :  return L"";
        }
    }

    // !! ИМЯ ФАЙЛА !!
    CertificateViewer::CertificateViewer(const wchar_t *filename)
        : m_target_filename(filename)
    {
        initialize();
        getProgramAndPublisherInfo();
        getTimestamp();
        getSignerInfo();
    }

    CertificateDetails CertificateViewer::getDetails() const
    {
        return m_details;
    }
}
