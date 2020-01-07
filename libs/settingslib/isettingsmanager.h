#pragma once

#include <common/aliases.h>

namespace drjuke::settingslib
{
    enum class SettingId
    {
        kProtectedFiles = 0
    };

    class ISettingsManager
    {
    public:
        virtual ~ISettingsManager()                       = default;
        virtual Json get(SettingId id)             = 0;
        virtual void set(SettingId id, Json value) = 0;
    };
}