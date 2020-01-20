#include "update_checker.h"

#pragma warning(push)
#pragma warning(disable:26451)
#include <json/json.hpp>
#pragma warning(pop)

namespace drjuke::netlib
{
    size_t UpdateChecker::on_http_data(void *buffer, size_t size, size_t nmemb, void *stream)
    try
    {
        const auto real_size = size * nmemb;
        static_cast<Responce*>(stream)->data += std::string(static_cast<char*>(buffer), real_size);
        return real_size;
    }
    catch (const std::exception& /*ex*/)
    {
        return 0;
    }

    std::map<std::string, std::pair<std::string, std::uint32_t>> UpdateChecker::getActualHashes() const
    {
        curl_easy_setopt(m_curl.get(), CURLOPT_URL, R"(http://127.0.0.1:9999)");
        curl_easy_setopt(m_curl.get(), CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, on_http_data);
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA, &m_responce);

        auto status = curl_easy_perform(m_curl.get());

        if (status != CURLE_OK)
        {
            throw std::runtime_error("curl failed");
        }

        Json responce = Json::parse(m_responce.data);
        std::map<std::string, std::pair<std::string, std::uint32_t>> result;

        for (auto it = responce.begin(); it != responce.end(); ++it)
        {
            result[it.key()] = std::make_pair(it.value()[0],
                                              it.value()[1]);
        }

        return result;
    }
}
