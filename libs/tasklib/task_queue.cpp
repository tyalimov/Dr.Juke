#include "task_queue.h"

#include <common/constants.h>

namespace drjuke::threading
{
    ITaskPtr TaskQueue::popTask()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // condition_variable может выдать ложное срабатывание,
        // поэтому устанавливаем предикат окончания ожидание.
        m_cv.wait(lock, [&]()
            {
                return m_finalized || !this->m_queue.empty();
            });

        if (m_finalized)
        {
            return std::make_shared<StubTask>();
        }

        auto task = m_queue.front();
        m_queue.pop();

        return task;
    }

    void TaskQueue::pushTask(ITaskPtr task)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (!m_finalized)
        {
            if (m_queue.size() <= constants::tasklib::kMaxTaskQueueSize)
            {
                m_queue.push(task);
                m_cv.notify_one();
            }
        }
        return;
    }

    void TaskQueue::finalize()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_finalized = true;
        m_cv.notify_all();
    }

    bool TaskQueue::isFinalized() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        return m_finalized;
    }
}
