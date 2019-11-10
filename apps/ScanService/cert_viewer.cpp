#include "cert_viewer.h"

#include <iostream>
#include <wintrust.h>

#include <memory>

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

namespace drjuke::scansvc
{
    void CertificateViewer::initialize()
    {
        ZeroMemory(&ProgPubInfo, sizeof(ProgPubInfo));

        fResult = CryptQueryObject
        (
            CERT_QUERY_OBJECT_FILE,
            m_target_filename.c_str(),
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
            CERT_QUERY_FORMAT_FLAG_BINARY,
            0,
            &dwEncoding,
            &dwContentType,
            &dwFormatType,
            &hStore,
            &hMsg,
            nullptr
        );

        if (!fResult)
        {
            std::cout << "CryptQueryObject failed with" << GetLastError() << std::endl;;
            return;
        }

        // Get signer information size.
        fResult = CryptMsgGetParam
        (
            hMsg, 
            CMSG_SIGNER_INFO_PARAM, 
            0, 
            nullptr, 
            &dwSignerInfo
        );

        if (!fResult)
        {
            std::cout << "CryptMsgGetParam failed with " << GetLastError() << std::endl;
            return;
        }

        // Allocate memory for signer information.
        pSignerInfo = static_cast<PCMSG_SIGNER_INFO>(LocalAlloc(LPTR, dwSignerInfo));

        if (!pSignerInfo)
        {
            std::cout << "Unable to allocate memory for Signer Info" << std::endl;
            return;
        }

        // Get Signer Information.
        fResult = CryptMsgGetParam
        (
            hMsg, 
            CMSG_SIGNER_INFO_PARAM, 
            0, 
            static_cast<PVOID>(pSignerInfo), 
            &dwSignerInfo
        );

        if (!fResult)
        {
            std::cout << "CryptMsgGetParam failed with " << GetLastError() << std::endl;
            return;
        }
    }

    void CertificateViewer::getProgramAndPublisherInfo()
    {
        PSPC_SP_OPUS_INFO opus_info = nullptr;  
        DWORD dwData{0};
        auto attribute = getNecessaryAttribute(pSignerInfo, SPC_SP_OPUS_INFO_OBJID);

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
        m_publisher_info.lpszProgramName =
            opus_info->pwszProgramName ?
            opus_info->pwszProgramName :
            L"";

        // Fill in Publisher Information if present.
        m_publisher_info.lpszPublisherLink =
            opus_info->pPublisherInfo ?
            getCertificateInfo(opus_info, CertificateInfo::kPublisherInfo) : 
            L"";

        // Fill in More Info if present.
        m_publisher_info.lpszMoreInfoLink =
            opus_info->pMoreInfo ?
            getCertificateInfo(opus_info, CertificateInfo::kMoreInfo) :
            L"";
    }

    void CertificateViewer::getDateOfTimestamps()
    {
        FILETIME local_filetime;
        FILETIME filetime;
        DWORD    info_size{sizeof(FILETIME)};

        auto attribute = getNecessaryAttribute(pSignerInfo, szOID_RSA_signingTime);

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
        FileTimeToSystemTime(&local_filetime, &m_date_of_timestamp);
    } 

    void CertificateViewer::getSignerTimestamps()
    {    
        DWORD dw_size{0};

        pCounterSignerInfo = nullptr;

        CRYPT_ATTRIBUTE attribute = getNecessaryAttribute(pSignerInfo, szOID_RSA_counterSign);

        // Get size of CMSG_SIGNER_INFO structure.
        fResult = CryptDecodeObject
        (
            ENCODING,
            PKCS7_SIGNER_INFO,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            nullptr,
            &dw_size
        );

        if (!fResult)
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
        fResult = CryptDecodeObject
        (
            ENCODING,
            PKCS7_SIGNER_INFO,
            attribute.rgValue[0].pbData,
            attribute.rgValue[0].cbData,
            0,
            static_cast<PVOID>(pCounterSignerInfo),
            &dw_size
        );

        if (!fResult)
        {
            std::cout << "CryptDecodeObject failed with " << GetLastError() << std::endl;
            return;
        }
    }

    CRYPT_ATTRIBUTE CertificateViewer::getNecessaryAttribute(PCMSG_SIGNER_INFO info, const std::string &object_id)
    {
        for (DWORD i = 0; i < info->UnauthAttrs.cAttr; i++)
        {
            auto obj_id = info->UnauthAttrs.rgAttr[i].pszObjId;

            if (std::string(obj_id) == object_id)
            {
                return info->UnauthAttrs.rgAttr[i];
            }
        }

        return CRYPT_ATTRIBUTE();
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
        getDateOfTimestamps();
        getSignerTimestamps();
    }

    CertificateDetails CertificateViewer::getDetails()
    {
        CertificateDetails details;

        return details;
    }
}
