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
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

    }
}

TEST(ipclib, connect_disconnect)
{
    //
    // Client and server try to establish duplex connection
    // If duplex connection is established then test is passed
    //

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
    //
    // Client and server exchange one message
    // If client and server recieved this message then test is passed
    //

    bool result_srv = false, result_cli = false;
    std::thread th_server(one_message::server_thread, std::ref(result_srv));
    std::thread th_client(one_message::client_thread, std::ref(result_cli));

    th_client.join();
    th_server.join();

    EXPECT_TRUE(result_cli);
    EXPECT_TRUE(result_srv);
    SUCCEED();
}

namespace multiple_messages
{
    std::list<Json> expected_msgs = {
        { "hello1" , "world1" },
        { "hello2" , "world2" },
        { "hello3" , "world3" },
        { "hello4" , "world4" },
        { "hello5" , "world5" },
    };

    void server_thread(bool& result)
    {
        std::list<Json> msgs = expected_msgs;

        try
        {
            auto server = Factory::getCommunicator(DirectionId::kServiceToGui);
            if (!server->connect())
                return;

			while (msgs.size() != 0)
			{
			    Json message = server->getMessage();
                Json expected_msg = msgs.front();

                std::cout << "Server: message recieved " << message.dump() << std::endl;
                if (message != expected_msg)
                    return;
                else
                    msgs.pop_front();
			}

            result = true;
            server->putMessage(expected_msgs.front());
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

            for (const auto& msg : expected_msgs)
		        client->putMessage(msg);

            // wait until server recieves all data or fails
			auto unused = client->getMessage();
            result = true;
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }

    }
}

TEST(ipclib, multiple_messages)
{
    //
    // Client sends to server multiple messages
    // If server receives all messages with initial order then test is passed
    //

    bool result_srv = false, result_cli = false;
    std::thread th_server(multiple_messages::server_thread, std::ref(result_srv));
    std::thread th_client(multiple_messages::client_thread, std::ref(result_cli));

    th_client.join();
    th_server.join();

    EXPECT_TRUE(result_cli);
    EXPECT_TRUE(result_srv);
    SUCCEED();
}

namespace disconnect
{
    void server_thread(bool& result, const CommunicatorPtr server)
    {

        Json msg = { "hello" , "world" };

        try
        {
            if (!server->connect())
                return;

            // empty json -> disconnect was called
            Json message = server->getMessage();
            if (message.size() == 0)
                result = true;

            server->putMessage(msg);
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
    
    void client_thread(bool& result, const CommunicatorPtr server)
    {
        try
        {
            Sleep(500);
            auto client = Factory::getCommunicator(DirectionId::kGuiToService);
            if (!client->connect())
                return;

            server->disconnect();
            auto unused = client->getMessage();
            result = true;
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
}

TEST(ipclib, disconnect)
{
    //
    // Server and client establish connection. Then client calls disconnect
    // Server recieves empty Json in getMessage and exits silently -> test is passed
    //

    bool result_srv = false, result_cli = false;
    auto server = Factory::getCommunicator(DirectionId::kServiceToGui);

    std::thread th_client(disconnect::client_thread, std::ref(result_cli), server);
    std::thread th_server(disconnect::server_thread, std::ref(result_srv), server);

    th_server.join();
    th_client.join();

    EXPECT_TRUE(result_cli);
    EXPECT_TRUE(result_srv);
    SUCCEED();
}

