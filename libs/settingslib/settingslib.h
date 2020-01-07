#pragma once

#include "isettingsmanager.h"

#include <memory>

namespace drjuke::settingslib
{
    using SettingsManagerPtr = std::unique_ptr<ISettingsManager>;

    class SettingsManagerFactory
    {
    public:
        static SettingsManagerPtr get();
    };
}