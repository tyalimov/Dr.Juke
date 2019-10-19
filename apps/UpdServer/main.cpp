#include <tasklib/tasklib.h>

#include <iostream>
#include <thread>
#include <string>

using namespace drjuke::threading;

class TestTask : public BasicTask
{
public:

	TestTask(const std::string& str)
		: BasicTask(Json({ {"str", "qwerty_" + str} }))
	{}

	void execute() override
	{
		//std::cout << std::this_thread::get_id() << " - " << m_input["str"].get<std::string>() << std::endl;
	}
};

void Producer(TaskQueue& queue)
{
	int i{ 0 };

	while (i++ < 100)
	{
		queue.pushTask(std::make_shared<TestTask>(TestTask(std::to_string(i))));
	}

	queue.finalize();
}

int main()
{
	TaskQueue queue;

	std::thread executor_thread_1(Executor, std::ref(queue));
	std::thread executor_thread_2(Executor, std::ref(queue));
	std::thread producer_thread(Producer, std::ref(queue));

	producer_thread.join();
	executor_thread_1.join();
	executor_thread_2.join();

	return 0;
}
