#pragma once

#include <undecorate.h>
#include <cstdint>
#include <filesystem>

#include "iupdate_checker.h"
#include "iupdater.h"

#pragma warning( push )                                   
#   pragma warning( disable : 4081 )                         
#   pragma comment(lib, UNDECORATE_LIBRARY("curl\\libcurl"))
#pragma warning( pop ) 

#define CURL_STATICLIB

namespace drjuke::netlib
{
    class NetlibFactory
    {
    public:
        [[nodiscard]] static UpdateCheckerPtr getUpdateChecker();
        [[nodiscard]] static UpdaterPtr       getUpdater();
    };

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
}