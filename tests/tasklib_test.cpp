#include "pch.h"

#include <tasklib/tasklib.h>

using namespace drjuke::threading;

class TestTask : public ITask
{
public:

    explicit TestTask(const drjuke::Json &input)
        : ITask(input)
    {}

    explicit TestTask()
        : ITask()
    {}

    void verifyInput() override
    {

    }

    void execute() override
    {
        std::cout << "k" << "e" << "k" << "\n";
    }
};

std::map<std::string, ITaskPtr> g_Tasks
{
    { "test_task", std::make_shared<TestTask>() }
};

class TestProducer : public IProducer
{
public:
    explicit TestProducer(TaskQueuePtr queue_ptr)
        : IProducer(queue_ptr)
    {}

    ITaskPtr constructTask(const drjuke::Json &input) override
    {
        auto task_id = input[drjuke::constants::tasklib::kJsonTaskId].get<std::string>();

        if (g_Tasks.find(task_id) == g_Tasks.end())
        {
            throw TaskNotFoundException();
        }

        return g_Tasks[task_id];
    }

    void listenEnvironment() override
    {
        int i = 0;
        while (i++ < 20)
        {
            drjuke::Json params;
            params["task_id"] = "test_task";
            auto task = constructTask(params);
            onTaskEvent(task);
        }

        m_queue->finalize();
    }
};

TEST(tasklib, Default) 
{
    TaskQueuePtr queue     = std::make_shared<TaskQueue>();
    ExecutorPtr  executor  = std::make_shared<Executor>(queue);
    IProducerPtr producer  = std::make_shared<TestProducer>(queue);

    std::thread executor_thread(ThreadExecutor, executor);
    std::thread producer_thread(ThreadProducer, producer);

    executor_thread.join();
    producer_thread.join();

    SUCCEED();
}