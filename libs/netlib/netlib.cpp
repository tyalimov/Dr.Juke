#include "netlib.h"
#include "update_checker.h"
#include "updater.h"
#include "uploader.h"

namespace drjuke::netlib
{
    UpdateCheckerPtr Factory::getUpdateChecker()
    {
        return std::make_unique<UpdateChecker>();
    }

    UploaderPtr Factory::getUploader(const Path& filename)
    {
        return std::make_unique<Uploader>(filename);
    }

    UpdaterPtr Factory::getUpdater(const std::vector<Path>& filenames, 
                                         const Path&              destination,
                                         LoadingProgressPtr           progress_bar)
    {
        return std::make_unique<Updater>(filenames, destination, progress_bar);
    }
}
