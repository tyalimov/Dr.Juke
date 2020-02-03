#include "ipc_listener.h"



namespace drjuke::service
{
    void IpcListenerThread::run()
    {
        while (true) try
        {
            auto message = m_communicator->getMessage();

            // Создать задачу на основе message
            // Положить задачу в m_queue

            /*
                Здесь нужно из списка задач брать по названию 
                задачи необходимую имплементацию
            */
        }
        catch (const std::exception& /*ex*/)
        {
            // TODO: Залогировать ошибку
        }
    }
}