#pragma once

#include "basic_task.h"
#include "task_queue.h"

#include <functional>

namespace drjuke::threading
{
    // Базовый класс, взаимодействующий
    // С очередью задач
    class QueueConsumer
    {
    protected:
		TaskQueuePtr m_queue_ptr;
    public:
        explicit QueueConsumer(TaskQueuePtr queue_ptr)
			: m_queue_ptr(queue_ptr)
		{}

		virtual ~QueueConsumer() = default;
    };

    class BasicExecutor : public QueueConsumer
    {		
    public:
		explicit BasicExecutor(TaskQueuePtr queue_ptr)
			: QueueConsumer(queue_ptr)
		{}

		virtual void executeTask(TaskPtr task) = 0;
    };

	class BasicProducer : public QueueConsumer
	{
	public:
		explicit BasicProducer(TaskQueuePtr queue_ptr, std::function<TaskQueuePtr> producer)
			: QueueConsumer(queue_ptr)
		{}

		virtual void executeTask(TaskPtr task) = 0;
	}; 
}