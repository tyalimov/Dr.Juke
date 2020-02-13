#include "ipc_listener.h"
#include "task_builder.h"

#include <winlib/winlib.h>


namespace drjuke::service
{
    void IpcListenerThread::run() try
    {
        WORKER_LOG("[Listener] started");
        WORKER_LOG("[Listener] waiting for GUI to connect...");

        auto status = m_communicator->connect();

        if (!status)
        {
            throw winlib::WindowsException("[Listener] failed connect to GUI");
        }

        WORKER_LOG("[Listener] GUI connected");

        while (true)
        {
            auto message = m_communicator->getMessage();

            WORKER_LOG("[Listener] Got new message: " + message.dump());

            auto task = TaskBuilder::buildTask(message);

            WORKER_LOG("[Listener] executing task with name: " + task->getName());
            auto responce = task->execute();

            WORKER_LOG("[Listener] Writing responce: " + responce.dump());
            m_communicator->putMessage(responce);
        }
    }
    catch (const std::exception& ex)
    {
        WORKER_LOG(ex.what());
        return;
    }
}
