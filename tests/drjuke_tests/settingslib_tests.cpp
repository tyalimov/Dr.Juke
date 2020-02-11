#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )

#include <settingslib/settingslib.h>

TEST(settingslib, aaa)
{
    auto mgr = drjuke::settingslib::Factory::getSettingsManager();

    mgr->addFirewallRule(L"firewall_rule_1", L"content_1");
    mgr->addFirewallRule(L"firewall_rule_2", L"content_2");

    mgr->addFilesystemFilterRule(L"filesystem_rule_1", 333);
    mgr->addFilesystemFilterRule(L"filesystem_rule_2", 444);

    mgr->addProcessFilterRule(L"process_rule_1", true);
    mgr->addProcessFilterRule(L"process_rule_2", false);

    mgr->addRegistryFilterRule(L"registry_rule_1", 333);
    mgr->addRegistryFilterRule(L"registry_rule_2", 444);

    std::cout << mgr->getFirewallRules() << std::endl;
    std::cout << mgr->getRegistryFilterRules() << std::endl;
    std::cout << mgr->getFilesystemFilterRules() << std::endl;
    std::cout << mgr->getProcessFilterRules() << std::endl;
}