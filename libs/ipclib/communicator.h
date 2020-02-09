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

        DWORD m_conn_timeout;
        DWORD m_read_timeout;
        RoleId m_role;

        //------------------------------------------------------------>
        // Private functions
        //------------------------------------------------------------>

        void clientInit();

        bool serverInit();

        bool waitForClientConnection(HANDLE hPipe, LPOVERLAPPED lpo);

    public:

        //------------------------------------------------------------>
        // Public functions
        //------------------------------------------------------------>

        explicit Communicator(const std::wstring& pipe_send, const std::wstring& pipe_recv,
            RoleId role, DWORD conn_timeout = INFINITE, DWORD read_timeout = INFINITE);

        ~Communicator();

        void putMessage(const Json &message) override;

        Json getMessage() override;

        virtual bool createDuplexConnection() override;

    };
}