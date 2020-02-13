#pragma once

#include <undecorate.h>
#include <filesystem>

#include "iupdate_checker.h"
#include "iupdater.h"
#include "iuploader.h"
#include "icloud_scanner.h"

LINK_LIBRARY("curl\\libcurl")

// Необходимо для нормальной линковки CURL в статическом режиме.
#define CURL_STATICLIB 

namespace drjuke::netlib
{
    struct LoadingProgress
    {
        Path   m_filename;
        size_t m_loaded;
        size_t m_total;

        LoadingProgress()
            : m_filename()
            , m_loaded(0)
            , m_total(0)
        {}

        LoadingProgress(const Path& filename, size_t file_size)
            : m_filename(filename)
            , m_loaded(0)
            , m_total(file_size)
        {}
    };

    using LoadingProgressPtr = std::shared_ptr<LoadingProgress>;
    
    class Factory
    {
    public:
        [[nodiscard]] static CloudScannerPtr  getCloudScanner();
        [[nodiscard]] static UpdateCheckerPtr getUpdateChecker();
        [[nodiscard]] static UploaderPtr      getUploader(const Path& filename);
        [[nodiscard]] static UpdaterPtr       getUpdater(const Path&        filename,
                                                         const Path&        destination,
                                                         LoadingProgressPtr progress_bar);
    };


}