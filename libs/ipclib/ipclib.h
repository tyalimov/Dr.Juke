#pragma once

#include "icommunicator.h"

namespace drjuke::ipclib
{
    class Factory
    {
    public:

        enum class QueueId
        { 
            kGuiToService,             
            kServiceToGui,   
            kReatimeToService,  
            kServiceToRealtime, 
        };

        static std::vector<std::string> m_queues_names;

        [[nodiscard]] static CommunicatorPtr getCommunicator(QueueId id);
    }; 
}