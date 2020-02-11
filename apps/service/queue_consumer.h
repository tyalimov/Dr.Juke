#pragma once

#include <tasklib/task_queue.h>

#include <mutex>
#include <iostream>

#define WORKER_LOG(message)                           \
    {                                                 \
        std::lock_guard<std::mutex> lock(m_console);  \
        std::cout << (message) << std::endl;          \
    }

namespace drjuke::service
{
    class QueueConsumerThread
    {
    protected:
        tasklib::TaskQueuePtr m_queue;
        std::mutex&           m_console;
    public:

        explicit QueueConsumerThread(tasklib::TaskQueuePtr queue, std::mutex& console)
            : m_queue(queue)
            , m_console(console)
        {}

        virtual void run()             = 0;
        virtual ~QueueConsumerThread() = default;
    };

    using QueueConsumerThreadPtr = std::unique_ptr<QueueConsumerThread>;

}
