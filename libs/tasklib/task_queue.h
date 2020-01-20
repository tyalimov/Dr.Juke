#pragma once

#include "i_task.h"

#include <queue>
#include <mutex>
#include <memory>

namespace drjuke::tasklib
{
    class TaskQueue
    {
    private:
        std::queue<BaseTaskPtr>  m_queue;
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

        BaseTaskPtr popTask();
        void pushTask(BaseTaskPtr task);
        void finalize();
        bool isFinalized();
    };

    using TaskQueuePtr = std::unique_ptr<TaskQueue>;
}
