#include "communicator.h"

#include <common/utils.h>

namespace drjuke::ipclib
{
    Communicator::Communicator(const std::string &queue_name)
        : m_queue
          (
              boost::interprocess::open_or_create_t(), 
              queue_name.c_str(), 
              ToUnderlying(Constants::kMaxQueueSize),
              ToUnderlying(Constants::kMaxMessageSize)
          )
    {
    }

    void Communicator::putMessage(const Json &message)
    {
        std::string serialized_data = message.dump();

        m_queue.send
        (
            serialized_data.c_str(), 
            serialized_data.size(), 
            ToUnderlying(Constants::kMaxMessageSize)
        );
    }

    Json Communicator::getMessage()
    {
        std::vector<char> buffer;
        buffer.reserve(ToUnderlying(Constants::kMaxMessageSize));

        size_t       received_size{ 0 };
        unsigned int priority{ 0 };

        m_queue.receive
        (
            buffer.data(), 
            ToUnderlying(Constants::kMaxMessageSize), 
            received_size, 
            priority
        );

        std::string serialized_data{ buffer.data(), received_size };

        return Json{ serialized_data };
    }
}
