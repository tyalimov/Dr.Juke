#include "task_builder.h"
#include "tasks_impl.h"

namespace drjuke::service
{
    enum class TaskId
    {
        kEnableFirewall,      // +
        kAddprotectedObject,  // ++
        kAddExclusion,        // ++
        kScan,                // --
        kCloudScan,           // --
        kAddFirewallRule,     // ++
        kRemoveFirewallRule,  // ++
        kEnableFirewallRule,  // ++
        kDisableFirewallRule  // ++
    };

    std::map<std::string, TaskId> TaskBuilder::m_ids
    {
        { "enable_firewall",       TaskId::kEnableFirewall      },
        { "add_protected_object",  TaskId::kAddprotectedObject  },
        { "add_exclusion",         TaskId::kAddExclusion        },
        { "scan",                  TaskId::kScan                },
        { "cloud_scan",            TaskId::kCloudScan           },
        { "add_firewall_rule",     TaskId::kAddFirewallRule     },
        { "remove_firewall_rule",  TaskId::kRemoveFirewallRule  },
        { "enable_firewall_rule",  TaskId::kEnableFirewallRule  },
        { "disable_firewall_rule", TaskId::kDisableFirewallRule },
    };


    BaseTaskPtr TaskBuilder::getTask(const Json& message)
    {
        auto name = message["task"].get<std::string>();
        auto id   = m_ids.at(name);

        switch (id)
        {
            case TaskId::kEnableFirewall      : return std::make_shared<tasks::EnableFirewall>(message);      break;
            case TaskId::kAddprotectedObject  : return std::make_shared<tasks::AddProtectedObject>(message);  break;
            case TaskId::kAddExclusion        : return std::make_shared<tasks::AddExclusion>(message);        break;
            case TaskId::kScan                : return std::make_shared<tasks::Scan>(message);                break;
            case TaskId::kCloudScan           : return std::make_shared<tasks::CloudScan>(message);           break;
            case TaskId::kAddFirewallRule     : return std::make_shared<tasks::AddFirewallRule>(message);     break;
            case TaskId::kRemoveFirewallRule  : return std::make_shared<tasks::RemoveFirewallRule>(message);  break;
            case TaskId::kEnableFirewallRule  : return std::make_shared<tasks::EnableFirewallRule>(message);  break;
            case TaskId::kDisableFirewallRule : return std::make_shared<tasks::DisableFirewallRule>(message); break;
        }
        return std::make_shared<tasklib::EndTask>();
    }
}
