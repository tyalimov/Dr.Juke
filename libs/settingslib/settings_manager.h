#pragma once

#include <string>
#include <common/winreg.h>

namespace drjuke::settingslib
{
    enum class KeyId;

    class SettingsManager 
    {
    private:
        [[nodiscard]] winreg::RegKey getKey(KeyId id) const;
        [[nodiscard]] bool isValueExists(const std::wstring& name, KeyId id) const;

        void deleteAllValues(KeyId id);
        void createAllKeys();

    public:
        void setDefaultSettings();

        // setters
        void setRootDirectory        (const std::wstring& directory);
        void setResourcesDirectory   (const std::wstring& directory);
        void setBinariesDirectory    (const std::wstring& directory);
        void setQuarantineDirectory  (const std::wstring& directory);
       
        // rule adders
        void addRegistryFilterRule   (const std::wstring& path, uint32_t access_mask);
        void addFilesystemFilterRule (const std::wstring& path, uint32_t access_mask);
        void addProcessFilterRule    (const std::wstring& path, bool access_mask);
        void addFirewallRule         (const std::wstring& name, const std::wstring& content);

        // special for firewall
        void enableFirewall      (bool enable);
        void enableFirewallRule  (const std::wstring& name);
        void disableFirewallRule (const std::wstring& name);

        // rule removers
        void removeRegistryFilterRule   (const std::wstring& name);
        void removeFilesystemFilterRule (const std::wstring& name);
        void removeProcessFilterRule    (const std::wstring& name);
        void removeFirewallRule         (const std::wstring& name);

        // exclusions for rules
        void excludeFromRegistryFilter   (const std::wstring& name, const std::wstring& path);
        void excludeFromFilesystemFilter (const std::wstring& name, const std::wstring& path);
        void excludeFromProcessFilter    (const std::wstring& name, const std::wstring& path);

        // getters
        [[nodiscard]] std::wstring getRootDirectory()        const;
        [[nodiscard]] std::wstring getResourcesDirectory()   const;
        [[nodiscard]] std::wstring getBinariesDirectory()    const;
        [[nodiscard]] std::wstring getQuarantineDirectory()  const;

        // cleaners
        void clearFilesystemFilterRules();
        void clearFirewallRulest();
        void clearRegistryFilterRules();
        void clearProcessFilterRules();
    };
}
