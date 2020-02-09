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

        void clientInit();

        bool serverInit();

        bool waitForClientConnection(HANDLE hPipe, LPOVERLAPPED lpo);

        Json validateJson(Json&& message);

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

        void putMessage(const Json &message) override;

        Json getMessage() override;

        virtual bool connect() override;
        virtual void disconnect() override;

    };
}