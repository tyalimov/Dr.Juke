#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )

#include <Windows.h>
#include <winlib/windows_exception.h>

#include <ipclib/ipclib.h>
#include <thread>

using namespace drjuke;
using namespace drjuke::ipclib;
using namespace drjuke::winlib;


namespace conn_disconn
{
    void server_thread(bool& result)
    {
        try
        {
            auto server = Factory::getCommunicator(DirectionId::kServiceToGui);
            result = server->connect();

            while (1)
            {
                Json message = server->getMessage();
                message;
            }
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

    }

    void client_thread(bool& result)
    {
        try
        {
            Sleep(500);
            auto client = Factory::getCommunicator(DirectionId::kGuiToService);
            result = client->connect();
        }
        catch (WindowsException & ex)
        {
            std::cout << ex.what() << std::endl;
        }

    }
}

TEST(ipclib, connect_disconnect)
{
    bool result_srv = false, result_cli = false;
    std::thread th_server(conn_disconn::server_thread, std::ref(result_srv));
    std::thread th_client(conn_disconn::client_thread, std::ref(result_cli));

    th_client.join();
    th_server.join();

    EXPECT_TRUE(result_cli);
    EXPECT_TRUE(result_srv);
    SUCCEED();
}

namespace one_message
{
    Json expected_msg = { "hello" , "world" };

    void server_thread(bool& result)
    {
        try
        {
            auto server = Factory::getCommunicator(DirectionId::kServiceToGui);
            if (!server->connect())
                return;

            while (1)
            {
                Json message = server->getMessage();
                if (message == expected_msg)
                {
                    result = true;
                    std::cout << "Server: recieve succeeded" << std::endl;
                    server->putMessage(expected_msg);
                }
            }
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

    }

    void client_thread(bool& result)
    {
        try
        {
            Sleep(500);
            auto client = Factory::getCommunicator(DirectionId::kGuiToService);
            if (!client->connect())
                return;

			client->putMessage(expected_msg);

			Json message = client->getMessage();
			if (message == expected_msg)
			{
				std::cout << "Client: recieve succeeded" << std::endl;
				result = true;
			}
			else
				std::cout << "Client: recieve failed" << std::endl;

        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

    }
}

TEST(ipclib, one_message)
{
    bool result_srv = false, result_cli = false;
    std::thread th_server(one_message::server_thread, std::ref(result_srv));
    std::thread th_client(one_message::client_thread, std::ref(result_cli));

    th_client.join();
    th_server.join();

    EXPECT_TRUE(result_cli);
    EXPECT_TRUE(result_srv);
    SUCCEED();
}
