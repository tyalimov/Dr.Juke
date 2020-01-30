#pragma once

#include "iupdater.h"
#include "libcurl.h"

#include <common/aliases.h>
#include <filesystem>

#include <loglib/loglib.h>

namespace drjuke::netlib
{
    class Updater final
        : public CurlConsumer    
        , public IUpdater
    {
    public:

        Updater(const std::vector<Path>& filenames,
                const Path&              destination,
                LoadingProgressPtr       progress_bar)
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
    
    private:
        DECLARE_CLASS_LOGGER();

        std::vector<Path>  m_filenames;
        Path               m_destination;
        LoadingProgressPtr m_progress_bar;
    };

    using UpdaterPtr = std::unique_ptr<IUpdater>;
}