#include "updater.h"
#include "curl_exception.h"

#include <winlib/filesys.h>

#include <iostream>

namespace drjuke::netlib
{
    IMPLEMENT_CLASS_LOGGER(Updater);

    void Updater::initialize()
    {
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, on_ftp_data);
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA,     m_progress_bar.get());
        curl_easy_setopt(m_curl.get(), CURLOPT_USERNAME,      "test");
        curl_easy_setopt(m_curl.get(), CURLOPT_PASSWORD,      "test");
    }

    void Updater::downloadFile()
    {
        LOG_TRACE(__FUNCTIONW__)

        auto url = R"(ftp://127.0.0.1:21/resources/)" + m_filename.generic_string();

        curl_easy_setopt(m_curl.get(), CURLOPT_URL, url.c_str());

        LOG_TRACE(L"Started URL: " + std::wstring(url.begin(), url.end()));

        if (fs::exists(m_filename))
        {
            winlib::filesys::deleteFile(m_filename);
        }

        auto status = curl_easy_perform(m_curl.get());

        if (status != CURLE_OK)
        {
            throw CurlException(status);
        }
    }

    size_t Updater::on_ftp_data(void *buffer, size_t size, size_t nmemb, void *stream)
    try
    {
        auto       progress_bar  = static_cast<LoadingProgress*>(stream);
        const auto real_size = size * nmemb;

        winlib::filesys::appendFile(progress_bar->m_filename, std::string(static_cast<char*>(buffer), real_size));

        progress_bar->m_loaded += real_size;

        LOG_TRACE(boost::wformat(L"## file=%s | income=%d | loaded=%d | total=%d")
            % progress_bar->m_filename
            % real_size
            % progress_bar->m_loaded
            % progress_bar->m_total);

        return real_size;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(ex.what());
        return 0;
    }
}
