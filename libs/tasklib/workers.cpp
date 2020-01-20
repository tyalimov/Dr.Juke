#include "workers.h"
#include "task_queue.h"

#include <iostream>

namespace drjuke::tasklib
{
    void Executor::listenQueue()
    {
        while (!m_queue->isFinalized())
        {
            m_queue->popTask()->execute();
        }
    }

    void IProducer::onTaskEvent(BaseTaskPtr task)
    {
        m_queue->pushTask(task);
    }

    void ThreadExecutor(ExecutorPtr executor)
    { 
        try
        {
            executor->listenQueue();
        }
        catch (const std::exception &ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void ThreadProducer(IProducerPtr producer)
    {
        try
        {
            producer->listenEnvironment();
        }
        catch (const std::exception &ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
}
