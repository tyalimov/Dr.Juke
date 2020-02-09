#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )

#include <memory>
#include <winlib/windows_exception.h>
#include <ipclib/ipclib.h>
#include <thread>

#include <ipclib/communicator.h>

using namespace drjuke;
using namespace drjuke::ipclib;
using namespace drjuke::winlib;

void basic_server(bool& result)
{
    try
    {
        auto server = Factory::getCommunicator(DirectionId::kServiceToGui);
        server->createDuplexConnection();
        result = true;
    }
    catch (const WindowsException& ex)
    {
        std::cout << ex.what() << std::endl;
        result = false;
    }

}

TEST(ipclib, connect_disconnect)
{
    bool result_srv, result_cli;
    std::thread th_server(basic_server, std::ref(result_srv));

    try
    {
        auto client = Factory::getCommunicator(DirectionId::kGuiToService);
        client->createDuplexConnection();
        result_cli = true;
    }
    catch (const WindowsException& ex)
    {
        std::cout << ex.what() << std::endl;
        result_cli = false;
    }

    th_server.join();

    EXPECT_TRUE(result_cli);
    EXPECT_TRUE(result_srv);
}
