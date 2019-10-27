#pragma once

#include "i_task.h"

#include <thread>
#include <queue>
#include <mutex>
#include <memory>

namespace drjuke::threading
{
    class TaskQueue
    {
    private:
        std::queue<ITaskPtr>      m_queue;
        std::condition_variable  m_cv;
        std::mutex               m_mutex;
        bool                     m_finalized;
      
    public:

        explicit TaskQueue()
            : m_queue()
            , m_cv()
            , m_mutex()
            , m_finalized(false)
        {}

        TaskQueue(const TaskQueue&) = delete;
        TaskQueue(TaskQueue &&rhs) = delete;

        ITaskPtr popTask();
        void pushTask(ITaskPtr task);
        void finalize();
        bool isFinalized();
    };

    using TaskQueuePtr = std::shared_ptr<TaskQueue>;
}
