#pragma once

#include "settings_manager.h"

#include <memory>

namespace drjuke::settingslib
{
    using SettingsManagerPtr = std::unique_ptr<SettingsManager>;

    class Factory
    {
    public:
        static SettingsManagerPtr getSettingsManager();
    };
}