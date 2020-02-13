#include "queue_executor.h"
#include "ipc_listener.h"

#include <tasklib/tasklib.h>

#include <thread>
#include <iostream>

int main() try
{
    std::mutex console_mutex;

    auto task_queue            = drjuke::tasklib::Factory::getTaskQueue();
    auto listener_communicator = drjuke::ipclib::Factory::getCommunicator(drjuke::ipclib::DirectionId::kServiceToGui);
    //auto executor_communicator = drjuke::ipclib::Factory::getCommunicator(drjuke::ipclib::DirectionId::kGuiToService);

    drjuke::service::IpcListenerThread   listener_thread { task_queue, console_mutex, listener_communicator };
    //drjuke::service::QueueExecutorThread executor_thread { task_queue, console_mutex, executor_communicator };

    std::thread ipc_listener   { &drjuke::service::IpcListenerThread::run,   &listener_thread  };
    //std::thread queue_executor { &drjuke::service::QueueExecutorThread::run, &executor_thread  };

    ipc_listener.join();
    //queue_executor.join();

    system("pause");

    return 0;
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    system("pause");
}
