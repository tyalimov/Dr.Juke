#pragma once

#include <common/aliases.h>
#include <common/win_raii.h>

namespace drjuke::settingslib
{
    enum class SettingId
    {
        kProtectedFiles     = 0,
        kResourcesDirectory = 1,
        kInstallDirectory   = 2
    };

    class ISettingsManager
    {
    public:
        virtual ~ISettingsManager()                = default;
        virtual Json get(SettingId id)             = 0;
        virtual void set(SettingId id, Json value) = 0;
        virtual void setDefaultSettings()          = 0; 
    };
}