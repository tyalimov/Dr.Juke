#include "task_queue.h"

#include <common.h>

#include <iostream>

namespace drjuke::threading
{
    TaskPtr TaskQueue::popTask()
    {
		std::unique_lock<std::mutex> lock(m_mutex);
		
        // condition_variable может выдать ложное срабатывание,
        // поэтому устанавливаем предикат окончания ожидание.
		m_cv.wait(lock, [&]()
			{
				return m_finalized || !this->m_queue.empty();
			});

		std::cout << std::this_thread::get_id() << " - pop_locked\n";

        if (m_finalized)
        {
			std::cout << std::this_thread::get_id() << " - finalized\n";
			return std::make_shared<StubTask>(StubTask());
        }

		
        auto task = m_queue.front();
		m_queue.pop();

		std::cout << std::this_thread::get_id() << " - pop_unlocked\n";

		return task;
    }

    void TaskQueue::pushTask(TaskPtr task)
    {
		std::unique_lock<std::mutex> lock(m_mutex);

		if (!m_finalized)
		{
			std::cout << std::this_thread::get_id() << " - push_locked\n";

			if (m_queue.size() <= Constants::kMaxTaskQueueSize)
			{
				m_queue.push(task);
				m_cv.notify_one();
			}

			std::cout << std::this_thread::get_id() << " - push_locked\n";
		}

        return;
    }

    void TaskQueue::finalize()
    {
		std::unique_lock<std::mutex> lock(m_mutex);

		std::cout << std::this_thread::get_id() << " - stopped_queue\n";

		m_finalized = true;

		m_cv.notify_all();
    }

    bool TaskQueue::isFinalized() 
    {
		std::unique_lock<std::mutex> lock(m_mutex);

		return m_finalized;
    }
}
