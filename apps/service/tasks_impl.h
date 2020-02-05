#pragma once

#include <tasklib/base_task.h>
#include <settingslib/settingslib.h>
#include <codecvt>

#define TASK_STD_CTOR(class_name)               \
    explicit class_name(const Json& message)    \
        : BaseTask(message) {}

#define TASK_NOT_END_FUNCTION                   \
    bool isEndTask() override { return false; } 

#define DECLARE_TASK(class_name)                \
    TASK_STD_CTOR(class_name)                   \
    TASK_NOT_END_FUNCTION

using namespace drjuke::tasklib;

namespace drjuke::service::tasks
{
#pragma warning ( push )
#pragma warning ( disable:4996 )

    inline std::wstring ToWstring(const std::string& text)
    {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>{"", L""}.from_bytes(text);
    }

#pragma warning (pop)

    class EnableFirewall final
        : public BaseTask
    {
    public:

        DECLARE_TASK(EnableFirewall)
        

        /*
            {
                "task"        : "enable_firewall",
                "parameters"  : 
                {
                    "name" : <уникальное имя правила>
                }
            }       
        */

        void execute() override
        {
            auto name = ToWstring(m_input["parameters"]["name"].get<std::string>());
            settingslib::Factory::getSettingsManager()->enableFirewallRule(name);
        }
    };

    class AddProtectedObject final
        : public BaseTask
    {
    private:

        struct ProtectedObjects { enum 
        {
            kFile    = 0,
            kKey     = 1,
            kProcess = 2
        }; };

    public:

        DECLARE_TASK(AddProtectedObject)
        
        /*
            {
                "task"        : "add_protected_object",
                "parameters"  : 
                {
                    "path"        : <путь>,
                    "type:        : { 0 - файл, 1 - ключ реестра, 2 - процесс }
                    "accecc_mask" : <число>  
                        В случае процесса - {0 - запрет, 1 - разрешение} 
                        В остальных случаях - маска доступа
                }
            }       
        */

        void execute() override
        {
            auto path        = ToWstring(m_input["parameters"]["path"].get<std::string>());
            auto type        = m_input["parameters"]["type"].get<int>();
            auto access_mask = m_input["parameters"]["access_mask"].get<int>();

            auto settings_manager = settingslib::Factory::getSettingsManager();

            switch (type)
            {
            case ProtectedObjects::kFile:    settings_manager->addFilesystemFilterRule(path, access_mask); break;
            case ProtectedObjects::kKey:     settings_manager->addRegistryFilterRule(path, access_mask); break;
            case ProtectedObjects::kProcess: settings_manager->addProcessFilterRule(path, access_mask); break;
            }
        }
    };

    class AddExclusion final
        : public BaseTask
    {
    private:

        struct Exclusions { enum 
        {
            kFile    = 0,
            kKey     = 1,
            kProcess = 2
        }; };

    public:

        DECLARE_TASK(AddExclusion)
        
        /*
            {
                "task"        : "add_exclusion",
                "parameters"  : 
                {
                    "exclusor_path"  : <путь к процессу>,
                    "exclusion_path" : <путь к исключаемому объекту>
                    "type: : { 0 - файл, 1 - ключ реестра, 2 - процесс } 
                }
            }       
        */

        void execute() override
        {
            auto excludor_path = ToWstring(m_input["parameters"]["exclusor_path"].get<std::string>());
            auto exclusion_path = ToWstring(m_input["parameters"]["exclusion_path"].get<std::string>());
            auto type = m_input["parameters"]["type"].get<int>();

            auto settings_manager = settingslib::Factory::getSettingsManager();

            switch (type)
            {
            case Exclusions::kFile:    settings_manager->excludeFromFilesystemFilter(excludor_path, exclusion_path); break;
            case Exclusions::kKey:     settings_manager->excludeFromRegistryFilter(excludor_path, exclusion_path); break;
            case Exclusions::kProcess: settings_manager->excludeFromProcessFilter(excludor_path, exclusion_path); break;
            }
        }
    };

    // TODO: реализовать
    class Scan final
        : public BaseTask
    {
    public:

        DECLARE_TASK(Scan)
        
        /*
            {
                "task"        : "scan",
                "parameters"  : 
                {
                    "path"  : <путь к файлу, который нужно просканировать>
                }
            }       
        */

        void execute() override
        {
        }
    };

    // TODO: реализовать
    class CloudScan final
        : public BaseTask
    {
    public:

        DECLARE_TASK(CloudScan)
        
        /*
            {
                "task"        : "cloud_scan",
                "parameters"  : 
                {
                    "path"  : <путь к файлу, который нужно просканировать>
                }
            }       
        */

        void execute() override
        {
        }
    };

    class AddFirewallRule final
        : public BaseTask
    {
    public:

        DECLARE_TASK(AddFirewallRule)
        

        /*
            {
                "task"        : "add_firewall_rule",
                "parameters"  : 
                {
                    "name"    : <уникальное имя правила>
                    "content" : <содержимое правила>
                }
            }    
            !!! Правило по умолчанию становится enabled !!!
        */

        void execute() override
        {
            auto name    = ToWstring(m_input["parameters"]["name"].get<std::string>());
            auto content = ToWstring(m_input["parameters"]["content"].get<std::string>());
            settingslib::Factory::getSettingsManager()->addFirewallRule(name, content);
        }
    };

    class RemoveFirewallRule final
        : public BaseTask
    {
    public:

        DECLARE_TASK(RemoveFirewallRule)
        

        /*
            {
                "task"        : "remove_firewall_rule",
                "parameters"  : 
                {
                    "name"    : <уникальное имя правила>
                }
            }    
            !!! Будет искать его и в enabled и в disabled !!!
        */

        void execute() override
        {
            auto name = ToWstring(m_input["parameters"]["name"].get<std::string>());
            settingslib::Factory::getSettingsManager()->enableFirewallRule(name);
        }
    };

    class EnableFirewallRule final
        : public BaseTask
    {
    public:

        DECLARE_TASK(EnableFirewallRule)
        

        /*
            {
                "task"        : "enable_firewall_rule",
                "parameters"  : 
                {
                    "name"    : <уникальное имя правила>
                }
            }    
        */

        void execute() override
        {
            auto name    = ToWstring(m_input["parameters"]["name"].get<std::string>());
            settingslib::Factory::getSettingsManager()->enableFirewallRule(name);
        }
    };


    class DisableFirewallRule final
        : public BaseTask
    {
    public:

        DECLARE_TASK(DisableFirewallRule)
        

        /*
            {
                "task"        : "disable_firewall_rule",
                "parameters"  : 
                {
                    "name"    : <уникальное имя правила>
                }
            }    
        */

        void execute() override
        {
            auto name    = ToWstring(m_input["parameters"]["name"].get<std::string>());
            settingslib::Factory::getSettingsManager()->disableFirewallRule(name);
        }
    };

    class AddToQuarantine final
        : public BaseTask
    {
    public:

        DECLARE_TASK(AddToQuarantine)
        

        /*
            {
                "task"        : "add_to_quarantine",
                "parameters"  : 
                {
                    "path"    : путь к файлу
                }
            }    
        */

        void execute() override
        {
            // Скопировать в папку карантина (дописать в сеттингслиб getQuarantineDirectory)
            // Удалить оттуда, откуда взял
        }
    };
}
