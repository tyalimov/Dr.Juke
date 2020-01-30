#pragma once

#include <undecorate.h>
#include <filesystem>

#include "iupdate_checker.h"
#include "iupdater.h"

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

        explicit LoadingProgress(const Path& filename)
            : m_filename(filename)
            , m_loaded(0)
            , m_total(fs::file_size(filename))
        {}
    };

    using LoadingProgressPtr = std::shared_ptr<LoadingProgress>;
    
    class Factory
    {
    public:
        [[nodiscard]] static UpdateCheckerPtr getUpdateChecker();
        [[nodiscard]] static UpdaterPtr       getUpdater(const std::vector<Path>& filenames,
                                                         const Path&              destination,
                                                         LoadingProgressPtr       progress_bar);
    };


}