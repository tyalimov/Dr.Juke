#pragma once

#include "base_task.h"

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
        bool                     m_stop;
      
    public:

        explicit TaskQueue()
            : m_queue()
            , m_cv()
            , m_mutex()
            , m_stop(false)
        {}

        BaseTaskPtr popTask();
        void pushTask(BaseTaskPtr task);
        void stop();
        bool isStopped();
    };

    using TaskQueuePtr = std::shared_ptr<TaskQueue>;
}
