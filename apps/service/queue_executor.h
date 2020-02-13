#pragma once

#include "queue_consumer.h"
#include "ipclib/ipclib.h"

namespace drjuke::service
{
    class QueueExecutorThread final
        : public QueueConsumerThread
    {
    private:
        ipclib::CommunicatorPtr m_communicator;
    public:
        explicit QueueExecutorThread(tasklib::TaskQueuePtr queue, std::mutex& console, ipclib::CommunicatorPtr communicator)
            : QueueConsumerThread(queue, console)
            , m_communicator(communicator)
        {}

        void run() override;
    };

}