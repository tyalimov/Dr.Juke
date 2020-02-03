#include "task_queue.h"


namespace drjuke::tasklib
{
    BaseTaskPtr TaskQueue::popTask()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // condition_variable может выдать ложное срабатывание,
        // поэтому устанавливаем предикат окончания ожидание.
        m_cv.wait(lock, [&]()
            {
                return m_stop || !this->m_queue.empty();
            });

        if (m_stop)
        {
            return std::make_shared<EndTask>();
        }

        auto task = m_queue.front();
        m_queue.pop();

        return task;
    }

    void TaskQueue::pushTask(BaseTaskPtr task)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (!m_stop)
        {
            m_queue.push(task);
            m_cv.notify_one();
        }
    }

    void TaskQueue::stop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_stop = true;
        m_cv.notify_all();
    }

    bool TaskQueue::isStopped() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        return m_stop;
    }
}
