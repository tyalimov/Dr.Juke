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
    struct ProgressBar
    {
        Path   filename;
        double percentage;

        ProgressBar()
            : filename()
            , percentage(0.0)
        {}

        explicit ProgressBar(Path filename_)
            : filename(filename_)
            , percentage(0.0)
        {}
    };

    using ProgressBarPtr = std::shared_ptr<ProgressBar>;
    
    class Factory
    {
    public:
        [[nodiscard]] static UpdateCheckerPtr getUpdateChecker();
        [[nodiscard]] static UpdaterPtr       getUpdater(const std::vector<Path>& filenames,
                                                         const Path&              destination,
                                                         ProgressBarPtr           progress_bar);
    };


}