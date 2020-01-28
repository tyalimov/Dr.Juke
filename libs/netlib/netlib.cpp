#include "netlib.h"
#include "update_checker.h"
#include "updater.h"

namespace drjuke::netlib
{
    UpdateCheckerPtr Factory::getUpdateChecker()
    {
        return std::make_unique<UpdateChecker>();
    }

    UpdaterPtr Factory::getUpdater(const std::vector<Path>& filenames, 
                                         const Path&              destination,
                                         ProgressBarPtr           progress_bar)
    {
        return std::make_unique<Updater>(filenames, destination, progress_bar);
    }
}
