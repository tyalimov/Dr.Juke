#include "isettingsmanager.h"
#include "settings_manager.h"

#include <map>
#include <string>
#include <utility>

#pragma warning( push , 0 )
#   include <json/json.hpp>
#pragma warning( pop )

namespace drjuke::settingslib
{

    namespace
    {
        
        const std::map<SettingId, std::pair<uint32_t, std::wstring>> kKeys
        {
            { SettingId::kProtectedFiles, { REG_BINARY, L"wwww" } }
        };
    }

    winreg::RegKey SettingsManager::getKey(SettingId id) 
    {
        // TODO: out of range
        auto key = kKeys.at(id);

        return winreg::RegKey{ HKEY_LOCAL_MACHINE, key.second };
    }

    Json SettingsManager::get(SettingId id)
    {
        auto key             = getKey(id);
        auto value           = key.GetBinaryValue(L"value");
        auto serialized_data = std::string(value.begin(), value.end());

        return Json(serialized_data);
    }

    void SettingsManager::set(SettingId id, Json value)
    {
        std::string dump = value.dump();
        std::vector<BYTE> serialized_data{dump.begin(), dump.end()};

        // TODO: exceptions
        auto key = getKey(id);
        key.DeleteValue(L"value");
        key.SetBinaryValue(L"value", serialized_data);
    }
}
