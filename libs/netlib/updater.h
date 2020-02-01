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

        Updater(const Path&         filename,
                const Path&         destination,
                LoadingProgressPtr  progress_bar)
            : m_filename(filename)
            , m_destination(destination)
            , m_progress_bar(progress_bar)
        {
            initialize();
        }

        Updater()
            : m_filename()
            , m_destination()
            , m_progress_bar()
        {}

        void downloadFile() override;

    private:

        void initialize();
        static size_t on_ftp_data(void* buffer, size_t size, size_t nmemb, void* stream);
    
    private:
        DECLARE_CLASS_LOGGER();

        Path               m_filename;
        Path               m_destination;
        LoadingProgressPtr m_progress_bar;
    };

    using UpdaterPtr = std::unique_ptr<IUpdater>;
}