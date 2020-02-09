#pragma once

#include "icommunicator.h"
#include <Windows.h>

namespace drjuke::ipclib
{
    enum class RoleId
    {
        kRoleClient,
        kRoleServer,
    };


    class Communicator final
        : public ICommunicator
    {

    private:

        //------------------------------------------------------------>
        // Private fields
        //------------------------------------------------------------>

        static constexpr uint32_t BUFSIZE = 512;

        HANDLE m_event = nullptr;
        HANDLE m_pipe_send = nullptr;
        HANDLE m_pipe_recv = nullptr;

        std::wstring m_pipe_send_name;
        std::wstring m_pipe_recv_name;

        DWORD m_client_timeout;
        RoleId m_role;

        //------------------------------------------------------------>
        // Private functions
        //------------------------------------------------------------>

        bool waitForClientConnection(HANDLE hPipe, LPOVERLAPPED lpo);

        Json validateJson(Json&& message);

        void clientInit();

        bool serverInit();

        void cleanup();

    public:

        //------------------------------------------------------------>
        // Public functions
        //------------------------------------------------------------>

        explicit Communicator(const std::wstring& pipe_send, 
            const std::wstring& pipe_recv, RoleId role, DWORD client_timeout = 1000);

        ~Communicator();

        // No need to implement now
        Communicator(const Communicator&) = delete;
        Communicator(Communicator&&) = delete;

        //
        // Blocks until recieve json message
        // Returns Json object if successfull
        // Returns empty Json object if disconnect 
        // was called at the moment of receiving
        //

        Json getMessage() override;

        //
        // Establishes connection with another node
        // Returnes <true> if connection is established
        // Returnes <false> if disconnect was called 
        // at the moment of establishing connection
        //

        virtual bool connect() override;

        //
        // Sets event to exit immediately
        // and not wait for operation completion
        //

        virtual void disconnect() override;

        //
        // Blocks until recieve json message.
        // Returns Json object if successfull.
        // Returns empty Json object if disconnect 
        // was called at the moment of receiving
        //

        void putMessage(const Json &message) override;

    };
}