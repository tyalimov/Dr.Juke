#include "ipclib.h"
#include "communicator.h"

#include <common/utils.h>

namespace drjuke::ipclib
{
    std::vector<std::string> Factory::m_queues_names
    {
        "GuiToService",      
        "ServiceToGui",   
        "RealtimeToService",  
        "ServiceToRealtime", 
    };

    CommunicatorPtr Factory::getCommunicator(QueueId id)
    {
        return std::make_unique<Communicator>(m_queues_names[ToUnderlying(id)]);
    }
}