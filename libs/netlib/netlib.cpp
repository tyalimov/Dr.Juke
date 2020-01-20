#include "netlib.h"
#include "update_checker.h"
#include "updater.h"

namespace drjuke::netlib
{
    UpdateCheckerPtr NetlibFactory::getUpdateChecker()
    {
        return std::make_unique<UpdateChecker>();
    }

    UpdaterPtr NetlibFactory::getUpdater()
    {
        return std::make_unique<Updater>();
    }
}
