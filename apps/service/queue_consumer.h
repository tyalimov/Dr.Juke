#pragma once

#include <tasklib/task_queue.h>

namespace drjuke::service
{
    class QueueConsumerThread
    {
    private:
        tasklib::TaskQueuePtr m_queue;
    public:

        explicit QueueConsumerThread(tasklib::TaskQueuePtr queue)
            : m_queue(queue)
        {}

        virtual void run()             = 0;
        virtual ~QueueConsumerThread() = default;
    };

    using QueueConsumerThreadPtr = std::unique_ptr<QueueConsumerThread>;

    inline void RunQueueConsumer(QueueConsumerThreadPtr thr)
    {
        thr->run();
    }
}
