#pragma once

#include <memory>
#include <common/aliases.h>

#pragma warning( push , 0 )
#   include <json/json.hpp>
#pragma warning( pop )

namespace drjuke::ipclib
{
    class ICommunicator
    {
    public:
        virtual ~ICommunicator()                     = default;
        virtual Json getMessage()                    = 0;
        virtual void putMessage(const Json& message) = 0;
        virtual bool connect()                       = 0;
        virtual void disconnect()                    = 0;
    };

    //constexpr 
    using CommunicatorPtr = std::shared_ptr<ICommunicator>;
}