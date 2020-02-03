#include "queue_executor.h"

namespace drjuke::service
{
    void QueueExecutorThread::run()
    {
        while (true) try
        {
            auto task = m_queue->popTask();

            // Очередь задач прекратила свою работу,
            // завершаем поток.
            if (task->isEndTask())
                break;

            task->execute();
        }
        catch (const std::exception& /*ex*/)
        {
            // TODO: Залогировать ошибку
        }
    }
}