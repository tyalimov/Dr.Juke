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
        curl_easy_setopt(m_curl.get(), CURLOPT_USERNAME,      "updater");
        curl_easy_setopt(m_curl.get(), CURLOPT_PASSWORD,      "updater");
    }

    void Updater::downloadFile()
    {
        LOG_TRACE(__FUNCTIONW__)

        auto local_path = m_destination / m_filename;
        auto url        = R"(ftp://192.168.0.105:21/)" + m_filename.generic_string();

        curl_easy_setopt(m_curl.get(), CURLOPT_URL, url.c_str());

        LOG_TRACE(L"Started URL: " + std::wstring(url.begin(), url.end()));

        if (fs::exists(local_path))
        {
            LOG_DEBUG(boost::wformat(L"Deleting = [%s]") % local_path.generic_wstring());
            winlib::filesys::deleteFile(local_path);
        }

        m_progress_bar->m_filename = local_path;

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

        LOG_DEBUG(boost::wformat(L"## writing data to [%s]") % progress_bar->m_filename.generic_wstring());

        auto directory_to_create = progress_bar->m_filename;
        directory_to_create = directory_to_create.remove_filename();

        fs::create_directories(directory_to_create);

        winlib::filesys::appendFile
        (
            progress_bar->m_filename, 
            std::string
            (
                static_cast<char*>(buffer), 
                real_size
            )
        );

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
