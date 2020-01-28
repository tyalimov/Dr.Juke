#include "settingslib.h"

#include "settings_manager.h"

namespace drjuke::settingslib
{
    SettingsManagerPtr Factory::getSettingsManager()
    {
        return std::make_unique<SettingsManager>();
    }
}
