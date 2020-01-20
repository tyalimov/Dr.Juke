#pragma once

#include "i_task.h"
#include "task_queue.h"

namespace drjuke::tasklib
{
    // Базовый класс, взаимодействующий с 
    // очередью задач. 
    class QueueConsumer
    {
    protected:
        TaskQueuePtr m_queue;
    public:
        explicit QueueConsumer(TaskQueuePtr queue_ptr)
            : m_queue(queue_ptr)
        {}

        virtual ~QueueConsumer() = default;
    };

    class Executor : public QueueConsumer
    {
    public:
        explicit Executor(TaskQueuePtr queue_ptr)
            : QueueConsumer(queue_ptr)
        {}

        void listenQueue();
    };

    using ExecutorPtr = std::shared_ptr<Executor>;

    // Базовый класс "генератора" задач. 
    // Генератор слушает события во внешней
    // среде (pipe, ioctl и т.д.) и при 
    // поступлении задачи кладет ее в 
    // очередь
    class IProducer : public QueueConsumer
    {
    public:
        explicit IProducer(TaskQueuePtr queue_ptr)
            : QueueConsumer(queue_ptr)
        {}

        // Вызывается из listenEnvironment
        // в тот момент, когда из канала данных была 
        // считана новая задача
        void onTaskEvent(BaseTaskPtr task);

        // Конструирует задачу, специфичную для унаследованного
        // класса (забирает из своего map по id задачи)
        // В случае отсутствия id в map выбрасывает 
        // исключение
        virtual BaseTaskPtr constructTask(const Json &input) = 0;

        // Вызывается после конструирования объекта, бесконечно
        // слушает события на pipe, и при получении задачи оттуда
        // вызывает constructTask, который передается в onTaskEvent
        virtual void listenEnvironment() = 0;
    };

    using IProducerPtr = std::shared_ptr<IProducer>;

    class TaskNotFoundException final : public std::exception
    {
    public:
        [[nodiscard]] const char *what() const override
        {
            return "cannot found task with specified id";
        }
    };

    void ThreadExecutor(ExecutorPtr executor);
    void ThreadProducer(IProducerPtr producer);
}