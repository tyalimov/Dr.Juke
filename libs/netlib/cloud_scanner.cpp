#include "cloud_scanner.h"
#include "curl_exception.h"

namespace drjuke::netlib
{
    IMPLEMENT_CLASS_LOGGER(CloudScanner);

    size_t CloudScanner::on_http_data(void *buffer, size_t size, size_t nmemb, void *stream)
    try
    {
        LOG_TRACE(__FUNCTIONW__)
        const auto real_size = size * nmemb;
        static_cast<Responce*>(stream)->data += std::string(static_cast<char*>(buffer), real_size);
        return real_size;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(ex.what());
        return 0;
    }

    Json CloudScanner::scanFile(const Path& filename) const
    {
        std::string post_field = filename.filename().generic_string();

        curl_easy_setopt(m_curl.get(), CURLOPT_URL,           R"(http://127.0.0.1:9999)");
        curl_easy_setopt(m_curl.get(), CURLOPT_POSTFIELDS,    post_field.c_str());
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, on_http_data);
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA,     &m_responce);

        auto status = curl_easy_perform(m_curl.get());

        if (status != CURLE_OK)
        {
            throw CurlException(status);
        }

        return Json::parse(m_responce.data);
    }
}
