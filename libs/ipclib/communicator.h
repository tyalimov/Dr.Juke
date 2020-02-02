#pragma once

#include "icommunicator.h"

#include <boost/interprocess/ipc/message_queue.hpp>

namespace drjuke::ipclib
{
    enum class Constants
    {
        kMaxQueueSize    = 1000000,
        kMaxMessageSize  = 100000,
        kDefaultPriority = 1
    };

    class Communicator final
        : public ICommunicator
    {
    private:
        boost::interprocess::message_queue m_queue;

    public:
        explicit Communicator(const std::string& queue_name);

        void putMessage(const Json &message) override;
        Json getMessage() override;
    };
}