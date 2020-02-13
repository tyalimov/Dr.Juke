#pragma once

#include "settings_manager.h"

#include <memory>

#pragma warning( push , 0 )
#   include <json/json.hpp>
#pragma warning( pop )

namespace drjuke::settingslib
{
    using SettingsManagerPtr = std::unique_ptr<SettingsManager>;

    class Factory
    {
    public:
        static SettingsManagerPtr getSettingsManager();
    };
}