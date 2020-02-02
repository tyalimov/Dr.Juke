#include "settings_manager.h"

#include <map>
#include <string>
#include <utility>
#include <sstream>

#pragma warning( push , 0 )
#   include <json/json.hpp>
#pragma warning( pop )

#define PROCESS_ACCESS_ALLOW 1
#define PROCESS_ACCESS_DENY  0
#define FIREWALL_ENABLE      1
#define FIREWALL_DISABLE     0

namespace drjuke::settingslib
{
    uint32_t HexToInt(const std::string& hex)
    {
        uint32_t x;   
        std::stringstream ss;
        ss << std::hex << hex;
        ss >> x;

        return x;
    }

    enum class KeyId
    {
        kRoot,
        kResources,
        kBinaries,
        kFilesystemObjects,
        kFilesystemExcluded,
        kRegistryObjects,
        kRegistryExcluded,
        kProcessObjects,
        kProcessExcluded,
        kFirewall,
        kFirewallEnabledRules,
        kFirewallDisabledRules
    };

    const std::map<KeyId, std::wstring> kPaths
    {
        { KeyId::kRoot,                    LR"(SOFTWARE\Dr.Juke\Paths\RootDirectory)"                      },
        { KeyId::kResources,               LR"(SOFTWARE\Dr.Juke\Paths\ResourcesDirectory)"                 },
        { KeyId::kBinaries,                LR"(SOFTWARE\Dr.Juke\Paths\BinariesDirectory)"                  },
        { KeyId::kFilesystemObjects,       LR"(SOFTWARE\Dr.Juke\FilesystemFilterRules\ProtectedObjects)"   },
        { KeyId::kFilesystemExcluded,      LR"(SOFTWARE\Dr.Juke\FilesystemFilterRules\ExcludedProcesses)"  },
        { KeyId::kRegistryObjects,         LR"(SOFTWARE\Dr.Juke\RegistryFilterRules\ProtectedObjects)"     },
        { KeyId::kRegistryExcluded,        LR"(SOFTWARE\Dr.Juke\RegistryFilterRules\ExcludedProcesses)"    },
        { KeyId::kProcessObjects,          LR"(SOFTWARE\Dr.Juke\ProcessMonitorRules\ProtectedObjects)"     },
        { KeyId::kProcessExcluded,         LR"(SOFTWARE\Dr.Juke\ProcessMonitorRules\ExcludedProcesses)"    },
        { KeyId::kFirewall,                LR"(SOFTWARE\Dr.Juke\FirewallRules\)"                           },
        { KeyId::kFirewallEnabledRules,    LR"(SOFTWARE\Dr.Juke\FirewallRules\EnabledRules)"               },
        { KeyId::kFirewallDisabledRules,   LR"(SOFTWARE\Dr.Juke\FirewallRules\DisabledRules)"              },
    };

    namespace default_values
    {
        const std::wstring kRootDirectory      { LR"(C:\Program Files\Dr.juke)"     };
        const std::wstring kResourcesDirectory { LR"(C:\Program Files\Dr.juke\res)" };
        const std::wstring kBinariesDirectory  { LR"(C:\Program Files\Dr.juke\bin)" };
    }

    // HKEY_LOCAL_MACHINE
    const std::vector<std::wstring> kRequiredKeys
    {
        LR"(SOFTWARE\Dr.Juke)",
        LR"(SOFTWARE\Dr.Juke\Paths)",
        LR"(SOFTWARE\Dr.Juke\FilesystemFilterRules)",
        LR"(SOFTWARE\Dr.Juke\RegistryFilterRules)",
        LR"(SOFTWARE\Dr.Juke\ProcessMonitorRules)",
        LR"(SOFTWARE\Dr.Juke\FirewallRules)",
        LR"(SOFTWARE\Dr.Juke\Paths\RootDirectory)",
        LR"(SOFTWARE\Dr.Juke\Paths\ResourcesDirectory)",
        LR"(SOFTWARE\Dr.Juke\Paths\BinariesDirectory)",
        LR"(SOFTWARE\Dr.Juke\FilesystemFilterRules\ProtectedObjects)",
        LR"(SOFTWARE\Dr.Juke\FilesystemFilterRules\ExcludedProcesses)",
        LR"(SOFTWARE\Dr.Juke\RegistryFilterRules\ProtectedObjects)",
        LR"(SOFTWARE\Dr.Juke\RegistryFilterRules\ExcludedProcesses)",
        LR"(SOFTWARE\Dr.Juke\ProcessMonitorRules\ProtectedObjects)",
        LR"(SOFTWARE\Dr.Juke\ProcessMonitorRules\ExcludedProcesses)",
        LR"(SOFTWARE\Dr.Juke\FirewallRules\EnabledRules)",
        LR"(SOFTWARE\Dr.Juke\FirewallRules\DisabledRules)",
    };

    //winreg::RegKey SettingsManager::getKey(SettingId id) 
    //{
    //    auto key = kKeys.at(id);

    //    return winreg::RegKey{ HKEY_LOCAL_MACHINE, key.second };
    //}

    //Json SettingsManager::get(SettingId id)
    //{
    //    auto key             = getKey(id);
    //    auto value           = key.GetBinaryValue(L"value");
    //    auto serialized_data = std::string(value.begin(), value.end());

    //    return Json(serialized_data);
    //}

    //void SettingsManager::set(SettingId id, Json value)
    //{
    //    std::string dump = value.dump();
    //    std::vector<BYTE> serialized_data{dump.begin(), dump.end()};

    //    // TODO: exceptions
    //    auto key = getKey(id);
    //    key.DeleteValue(L"value");
    //    key.SetBinaryValue(L"value", serialized_data);
    //}

    winreg::RegKey SettingsManager::getKey(KeyId id) const
    {
        return winreg::RegKey(HKEY_LOCAL_MACHINE, kPaths.at(id));
    }

    void SettingsManager::createAllKeys()
    {
        for (const auto& key : kRequiredKeys)
        {
            winreg::RegKey().Create(HKEY_LOCAL_MACHINE, key, KEY_ALL_ACCESS);
        }
    }

    void SettingsManager::setDefaultSettings()
    {
        createAllKeys();
        setRootDirectory(default_values::kRootDirectory);
        setResourcesDirectory(default_values::kResourcesDirectory);
        setBinariesDirectory(default_values::kBinariesDirectory);
        enableFirewall(true);
    }

    void SettingsManager::setRootDirectory(const std::wstring& directory)
    {
        getKey(KeyId::kRoot).SetStringValue(L"value", directory);
    }

    void SettingsManager::setResourcesDirectory(const std::wstring &directory)
    {
        getKey(KeyId::kResources).SetStringValue(L"value", directory);
    }

    void SettingsManager::setBinariesDirectory(const std::wstring &directory)
    {
        getKey(KeyId::kBinaries).SetStringValue(L"value", directory);
    }

    //{
    //    "Operation"     : "<--->"
    //    ProtectedObjects: 
    //    [
    //        {
    //            KeyPath  : "<--->",
    //            AccessMask : "0xFFFFFFFF"
    //        },
    //        {
    //            KeyPath  : "<--->",
    //            AccessMask : "0xFFFFFFFF"
    //        }
    //    ]
    //}
    void SettingsManager::addRegistryFilterRule(const std::wstring &path, uint32_t access_mask)
    {
        getKey(KeyId::kRegistryObjects).SetDwordValue(path, access_mask);
    }

    //{
    //    "Operation"     : "<--->"
    //    ProtectedObjects: 
    //    [
    //        {
    //            ImagePath  : "<--->",
    //            AccessMask : "0xFFFFFFFF"
    //        },
    //        {
    //            ImagePath  : "<--->",
    //            AccessMask : "0xFFFFFFFF"
    //        }
    //    ]
    //}
    void SettingsManager::addFilesystemFilterRule(const std::wstring &path, uint32_t access_mask)
    {
        getKey(KeyId::kFilesystemObjects).SetDwordValue(path, access_mask);
    }

    //{
    //    "Operation"     : "<--->"
    //    ProtectedObjects: 
    //    [
    //        {
    //            ImagePath  : "<--->",
    //            AccessMask : true / false
    //        },
    //        {
    //            ImagePath  : "<--->",
    //            AccessMask : true / false
    //        }
    //    ]
    //}
    void SettingsManager::addProcessFilterRule(const std::wstring& path, bool access_mask)
    {
        getKey(KeyId::kProcessObjects).SetDwordValue(path, access_mask ? PROCESS_ACCESS_ALLOW : PROCESS_ACCESS_DENY);
    }

    void SettingsManager::addFirewallRule(const std::wstring& name, const std::wstring& content)
    {
        getKey(KeyId::kFirewallEnabledRules).SetStringValue(name, content);
    }

    void SettingsManager::enableFirewall(bool enable)
    {
        getKey(KeyId::kFirewall).SetDwordValue(L"EnableFirewall", enable ? FIREWALL_ENABLE : FIREWALL_DISABLE);
    }

    void SettingsManager::removeRegistryFilterRule(const std::wstring& name)
    {
        getKey(KeyId::kRegistryObjects).DeleteValue(name);
    }

    void SettingsManager::removeFilesystemFilterRule(const std::wstring& name)
    {
        getKey(KeyId::kFilesystemObjects).DeleteValue(name);
    }

    void SettingsManager::removeProcessFilterRule(const std::wstring& name)
    {
        getKey(KeyId::kProcessObjects).DeleteValue(name);
    }

    void SettingsManager::removeFirewallRule(const std::wstring& name)
    {
        // Необходимо искать значение в обоих ключах.
        
        auto key_enabled        = getKey(KeyId::kFirewallEnabledRules);
        auto key_disabled       = getKey(KeyId::kFirewallDisabledRules);
        auto key_enabled_rules  = key_enabled.EnumValues();
        auto key_disabled_rules = key_disabled.EnumValues();

        auto iter_enabled = std::find_if(key_enabled_rules.begin(), key_enabled_rules.end(),
            [&name](const std::pair<std::wstring, DWORD>& val)
        {
            return val.first == name;
        });

        auto iter_disabled = std::find_if(key_disabled_rules.begin(), key_disabled_rules.end(),
            [&name](const std::pair<std::wstring, DWORD>& val)
        {
            return val.first == name;
        });

        if (iter_enabled != key_enabled_rules.end())
        {
            key_enabled.DeleteValue(name);
        }
                
        if (iter_disabled != key_disabled_rules.end())
        {
            key_disabled.DeleteValue(name);
        }
    }

    void SettingsManager::excludeFromRegistryFilter(const std::wstring& name, const std::wstring &path)
    {
        getKey(KeyId::kRegistryExcluded).SetStringValue(name, path);
    }

    void SettingsManager::excludeFromProcessFilter(const std::wstring &name, const std::wstring &path)
    {
        getKey(KeyId::kProcessExcluded).SetStringValue(name, path);
    }

    void SettingsManager::excludeFromFilesystemFilter(const std::wstring& name, const std::wstring &path)
    {
        getKey(KeyId::kFilesystemExcluded).SetStringValue(name, path);
    }

    std::wstring SettingsManager::getRootDirectory() const
    {
        return getKey(KeyId::kRoot).GetStringValue(L"value");
    }

    std::wstring SettingsManager::getResourcesDirectory() const
    {
        return getKey(KeyId::kResources).GetStringValue(L"value");
    }

    std::wstring SettingsManager::getBinariesDirectory() const
    {
        return getKey(KeyId::kBinaries).GetStringValue(L"value");
    }

    void SettingsManager::clearFilesystemFilterRules()
    {
        auto key    = getKey(KeyId::kFilesystemObjects);
        auto values = key.EnumValues();

        for (const auto& [first, second] : values)
        {
            key.DeleteValue(first);
        }
    }

    void SettingsManager::clearFirewallRulest()
    {
        auto key    = getKey(KeyId::kFilesystemObjects);
        auto values = key.EnumValues();

        for (const auto& [first, second] : values)
        {
            key.DeleteValue(first);
        }
    }

    void SettingsManager::clearRegistryFilterRules()
    {
        auto key    = getKey(KeyId::kRegistryObjects);
        auto values = key.EnumValues();

        for (const auto& [first, second] : values)
        {
            key.DeleteValue(first);
        }
    }

    void SettingsManager::clearProcessFilterRules()
    {
        auto key = getKey(KeyId::kProcessObjects);
        auto values = key.EnumValues();

        for (const auto& [first, second] : values)
        {
            key.DeleteValue(first);
        }
    }
}
