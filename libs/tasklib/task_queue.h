#pragma once

#include "basic_task.h"

#include <memory>

namespace drjuke::threading
{
	class TaskQueue
	{
	private:
		std::queue<TaskPtr>      m_queue;
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

		TaskPtr popTask();
		void pushTask(TaskPtr task);
		void finalize();
		bool isFinalized();
	};

	using TaskQueuePtr = std::shared_ptr<TaskQueue>;
}
