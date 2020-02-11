#include "queue_executor.h"
#include <winlib/winlib.h>
#include <iostream>

namespace drjuke::service
{
    void QueueExecutorThread::run() try
    {
        WORKER_LOG("[Executor] started");
        WORKER_LOG("[Executor] connecting to GUI");
        
        auto status = m_communicator->connect();

        if (!status)
        {
            throw winlib::WindowsException("[Executor] failed connect to GUI");
        }

        WORKER_LOG("[Executor] connected to GUI");

        while (true)
        {
            auto task = m_queue->popTask();

            WORKER_LOG("[Executor] got new task: " + task->getName());

            // Очередь задач прекратила свою работу,
            // завершаем поток.
            if (task->isEndTask())
            {
                WORKER_LOG("[Executor] finished");
                return;
            }

            WORKER_LOG("[Executor] executing: " + task->getName());
            task->execute();
        }
    }
    catch (const std::exception& ex)
    {
        WORKER_LOG(ex.what());
    }
}
