#pragma once

#include "task_queue.h"
#include "workers.h"

#include <vector>

namespace drjuke::threading
{
	class ThreadPool
	{
	private:
		TaskQueue m_queue;
		std::vector<QueueConsumer> m_consumers;

	public:
		void start();
	};
}