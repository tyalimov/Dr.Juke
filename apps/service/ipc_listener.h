#pragma once

#include "queue_consumer.h"

#include <tasklib/task_queue.h>
#include <ipclib/ipclib.h>

#include <utility>

namespace drjuke::service
{
    class IpcListenerThread final
        : public QueueConsumerThread
    {
    private:
        ipclib::CommunicatorPtr m_communicator;

    public:
        explicit IpcListenerThread(tasklib::TaskQueuePtr   queue,
                                   ipclib::CommunicatorPtr communicator)
            : QueueConsumerThread(queue)
            , m_communicator(std::move(communicator))
        {}

        void run() override;
    };
}