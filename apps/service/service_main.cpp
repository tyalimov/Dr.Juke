#include <windows.h>
#include <string>

#include <winlib/winlib.h>
#include <filesystem>

#if 0
  #pragma comment(lib, "advapi32.lib")

#pragma region Globals

std::wstring g_ServiceName
{
    L"DrJukeAVService" 
};

SERVICE_STATUS g_ServiceStatus
{
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_START_PENDING,
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
        0,
        0,
        0,
        0
};

SERVICE_STATUS_HANDLE g_ServiceStatusHandle
{
    nullptr
};

#pragma endregion

void WINAPI ControlHandler(DWORD request)
{
    switch (request)
    {
    case SERVICE_CONTROL_STOP: [[fallthrough]];// Здесь убить потоки
    case SERVICE_CONTROL_SHUTDOWN             : g_ServiceStatus.dwCurrentState = SERVICE_STOPPED; break; // И здесь убить потоки
    case SERVICE_CONTROL_PAUSE                : g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;  break;
    case SERVICE_CONTROL_CONTINUE             : g_ServiceStatus.dwCurrentState = SERVICE_RUNNING; break;
    case SERVICE_CONTROL_INTERROGATE          : break;
    default                                   : break;
    }

    SetServiceStatus
    (
        g_ServiceStatusHandle, 
        &g_ServiceStatus
    );
}

VOID WINAPI ServiceMain(int, char **)
{
    g_ServiceStatusHandle = RegisterServiceCtrlHandlerW
    (
        g_ServiceName.data(),
        static_cast<LPHANDLER_FUNCTION>(ControlHandler)
    );


    if (!g_ServiceStatusHandle)
    {
        return;
    }

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    
    auto status = SetServiceStatus
    (
        g_ServiceStatusHandle,
        &g_ServiceStatus
    );

            OutputDebugStringW
        (
            L"FHJDHKSDHKJSAHDKADHKA"
        );

    if (!status)
    {
        OutputDebugStringW
        (
            L"ServiceMain: SetServiceStatus returned error"
        );
    }

    auto desctop = drjuke::winlib::filesys::getDesktopDirectory();
    desctop /= "service_log.txt";

    drjuke::winlib::filesys::createFile(desctop);
    drjuke::winlib::filesys::appendFile(desctop, "Hello world");
}
#endif










#include <memory>
#include "queue_executor.h"
#include "ipc_listener.h"

#include <tasklib/tasklib.h>

#include <thread>

int main(int argc, char *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    auto task_queue            = drjuke::tasklib::Factory::getTaskQueue();
    auto listener_communicator = drjuke::ipclib::Factory::getCommunicator(drjuke::ipclib::DirectionId::kServiceToGui);

    std::thread ipc_listener   { drjuke::service::RunIpcListener, task_queue, listener_communicator };
    std::thread queue_executor { drjuke::service::RunQueueExecutor, task_queue                      };

    ipc_listener.join();
    queue_executor.join();

    return 0;
}