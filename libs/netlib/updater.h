#pragma once

#include "iupdater.h"
#include "libcurl.h"

#include <common/aliases.h>
#include <filesystem>

#include <loglib/loghlp.h>

namespace drjuke::netlib
{
    class Updater final
        : public CurlConsumer    
        , public IUpdater
    {
    public:

        struct FtpFile
        {
            std::size_t    downloaded;
            std::size_t    actual_size;
            ProgressBarPtr progress_bar;
            Path           filename;

            FtpFile(const std::string& filename_,
                    ProgressBarPtr     progress_bar_,
                    std::uint32_t      actual_size_)
                : downloaded(0)
                , actual_size(actual_size_)
                , progress_bar(progress_bar_)
                , filename(filename_)
            {}

            FtpFile()
                : downloaded(0)
                , actual_size(0)
                , progress_bar()
                , filename()
            {}
        };

        Updater(const std::vector<Path>& filenames,
                const Path&              destination,
                ProgressBarPtr           progress_bar)
            : m_filenames(filenames)
            , m_destination(destination)
            , m_progress_bar(progress_bar)
        {
            initialize();
        }

        Updater()
            : m_filenames()
            , m_destination()
            , m_progress_bar()
        {}

        void downloadFiles() override;

    private:

        void initialize();
        void downloadFile(const std::string& filename);
        static size_t on_ftp_data(void* buffer, size_t size, size_t nmemb, void* stream);

        std::vector<Path> m_filenames;
        Path              m_destination;
        ProgressBarPtr    m_progress_bar;
        FtpFile           m_current_download;
    };

    using UpdaterPtr = std::unique_ptr<IUpdater>;
}