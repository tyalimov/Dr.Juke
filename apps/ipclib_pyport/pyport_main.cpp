#define BOOST_PYTHON_STATIC_LIB

#include <Windows.h>
#include <boost/python.hpp>
#pragma warning (push, 0)
#include <ipclib/ipclib.h>
#include <iostream>
#pragma warning (pop)

#define UNUSED(x) (void)x

#pragma warning (disable:4996)

extern "C" __declspec(dllexport) void onApiCall(void* x)
{
    UNUSED(x);
}

drjuke::ipclib::CommunicatorPtr g_Communicator;

void Initialize() try
{
    std::cout << "Connecting to server...\n";
    g_Communicator = drjuke::ipclib::Factory::getCommunicator(drjuke::ipclib::DirectionId::kGuiToService);
    
    auto result_1 = g_Communicator->connect();
    
    if (!result_1)
    {
        std::cout << "Error connecting to server\n";
        return;
    }

    std::cout << "Connected to server\n";

}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    return;
}

std::string PopMessage() try
{
    drjuke::Json msg = g_Communicator->getMessage();
    return msg.dump();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    return "";
}

void PushMessage(std::string msg) try
{
    drjuke::Json message = drjuke::Json::parse(msg);
    g_Communicator->putMessage(message);
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    return;
}

BOOST_PYTHON_MODULE(ipclib_pyport)
{
    using namespace boost::python;
    def("PopMessage",  PopMessage);
    def("PushMessage", PushMessage);
}

BOOL APIENTRY DllMain
(
    HINSTANCE /*hinstDLL*/,
    DWORD     fdwReason, 
    LPVOID    /*lpvReserved*/
)
{
    

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH : Initialize(); return TRUE;
        case DLL_PROCESS_DETACH : return TRUE;
        case DLL_THREAD_ATTACH  : return TRUE;
        case DLL_THREAD_DETACH  : return TRUE;
        default                 : return TRUE;
    }
}