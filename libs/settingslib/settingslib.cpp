#include "settingslib.h"

#include "settings_manager.h"

namespace drjuke::settingslib
{
    SettingsManagerPtr SettingsManagerFactory::get()
    {
        return std::make_unique<SettingsManager>();
    }
}
