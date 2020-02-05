#pragma once

#include "queue_consumer.h"

namespace drjuke::service
{
    class QueueExecutorThread final
        : public QueueConsumerThread
    {
    private:
        tasklib::TaskQueuePtr m_queue;
    public:
        explicit QueueExecutorThread(tasklib::TaskQueuePtr queue)
            : QueueConsumerThread(queue)
        {}

        void run() override;
    };

    void RunQueueExecutor(tasklib::TaskQueuePtr queue);
}