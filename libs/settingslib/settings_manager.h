#pragma once

#include "isettingsmanager.h"
#include "winreg.h"

#include <map>
#include <string>
#include <common/utils.h>

namespace drjuke::settingslib
{
    class SettingsManager final
        : public ISettingsManager
    {
    private:
        static winreg::RegKey getKey(SettingId id);

    public:
        Json get(SettingId id)             override final;
        void set(SettingId id, Json value) override final;
    };
}
