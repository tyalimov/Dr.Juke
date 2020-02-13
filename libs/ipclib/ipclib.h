#pragma once

#include "icommunicator.h"

namespace drjuke::ipclib
{

    enum class DirectionId
    { 
        kGuiToService,             
        kServiceToGui,   
        kReatimeToService,  
        kServiceToRealtime,
        kScanToService,
        kServiceToScan,
    };

    class Factory
    {

    public:

        [[nodiscard]] static CommunicatorPtr getCommunicator(DirectionId id);

    }; 
}