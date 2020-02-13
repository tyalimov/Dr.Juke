#include "uploader.h"

#include "curl_exception.h"

const std::string kUrl           { R"(ftp://192.168.0.105:21/)" };
const std::string kUploadCommand { "RNFR " };

#pragma warning (disable:4996)

namespace drjuke::netlib
{

    IMPLEMENT_CLASS_LOGGER(Uploader);

    Uploader::Uploader(const Path &file)
        : m_file_name(file)
        , m_file_ptr(fopen(m_file_name.generic_string().c_str(), "rb"))
        , m_header_list(curl_slist_append(nullptr, (kUploadCommand + m_file_name.generic_string()).c_str()))
        , m_url(kUrl + m_file_name.filename().generic_string())
    {
        if (!m_file_ptr.get())
        {
            throw std::runtime_error("can't open file");
        }

        if (!m_header_list.get())
        {
            throw std::runtime_error("can't add command to curl");
        }

        curl_easy_setopt(m_curl.get(), CURLOPT_URL,              m_url.c_str());
        curl_easy_setopt(m_curl.get(), CURLOPT_READFUNCTION,     on_read_data);
        curl_easy_setopt(m_curl.get(), CURLOPT_UPLOAD,           1L);
        curl_easy_setopt(m_curl.get(), CURLOPT_POSTQUOTE,        m_header_list.get());
        curl_easy_setopt(m_curl.get(), CURLOPT_READDATA,         m_file_ptr.get());
        curl_easy_setopt(m_curl.get(), CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(fs::file_size(m_file_name)));
        curl_easy_setopt(m_curl.get(), CURLOPT_USERNAME,         "uploader");
        curl_easy_setopt(m_curl.get(), CURLOPT_PASSWORD,         "uploader");

        LOG_TRACE(L"Initialized Uploader");
        LOG_DEBUG(boost::wformat(L"file=%s") % m_file_name.generic_wstring());
        LOG_DEBUG(boost::wformat(L"url=%s") % std::wstring(m_url.begin(), m_url.end()));
    }

    void Uploader::upload()
    {
        auto status = curl_easy_perform(m_curl.get());

        // Костыль, так как libcurl возвращает 21 ошибку при успешной загрузке
        if (status != CURLE_OK && status != 21) 
        {
            LOG_ERROR(boost::wformat(L"curl failed with: %d") % static_cast<int>(status));
            throw CurlException(status);
        }
    }

    size_t Uploader::on_read_data(void *ptr, size_t size, size_t nmemb, void *stream)
    {
        return fread(ptr, size, nmemb, static_cast<FILE*>(stream));
    }
}
