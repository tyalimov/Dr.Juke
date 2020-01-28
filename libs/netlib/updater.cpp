#include "updater.h"

#include <winlib/filesys.h>

#include <iostream>

namespace drjuke::netlib
{
    void Updater::downloadFiles()
    {
        for (const auto& file : m_filenames)
        {
            downloadFile(file.generic_string());
        }
    }

    void Updater::initialize()
    {
        curl_easy_setopt(m_curl.get(), CURLOPT_VERBOSE,       1L);
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, on_ftp_data);
        curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA,     &m_current_download);
        curl_easy_setopt(m_curl.get(), CURLOPT_USERNAME,      "test");
        curl_easy_setopt(m_curl.get(), CURLOPT_PASSWORD,      "test");
    }

    void Updater::downloadFile(const std::string &filename)
    {
        auto url = R"(http://127.0.0.1:21/)" + filename;

        curl_easy_setopt(m_curl.get(), CURLOPT_URL, url.c_str());

        // TODO: loglib
        std::cout << "Started URL=" << url << std::endl;

        *m_progress_bar = ProgressBar(filename);

        curl_easy_perform(m_curl.get());
    }

    size_t Updater::on_ftp_data(void *buffer, size_t size, size_t nmemb, void *stream)
    try
    {
        auto       ftp_file  = static_cast<FtpFile*>(stream);
        const auto real_size = size * nmemb;

        winlib::filesys::AppendFile(ftp_file->filename,
                                    std::string(static_cast<char*>(buffer), real_size));

        ftp_file->downloaded               += real_size;
        ftp_file->progress_bar->percentage =  double(ftp_file->downloaded) / double(ftp_file->actual_size);

        std::cout
            << "loaded: "              << ftp_file->downloaded               << std::endl
            << "total: "               << ftp_file->actual_size              << std::endl
            << "percentage: "          << ftp_file->progress_bar->percentage << std::endl
            << "---------------------" << std::endl;
        
        return real_size;
    }
    catch (...)
    {
        std::cout << "something went wrong" << std::endl;
        return 0;
    }
}
