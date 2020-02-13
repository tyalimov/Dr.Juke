#include "communicator.h"

#include <Windows.h>

#include "winlib/windows_exception.h"
#include "winlib/raii.h"

namespace drjuke::ipclib
{

	Communicator::Communicator(const std::wstring& pipe_send,
		const std::wstring& pipe_recv, RoleId role, DWORD client_timeout)
	{
		m_role = role;
		m_pipe_recv_name = pipe_recv;
		m_pipe_send_name = pipe_send;
		m_client_timeout = client_timeout;

		m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_event == NULL)
			throw winlib::WindowsException("Create event failed");
	}

	Communicator::~Communicator() {
		cleanup();
	}

	void Communicator::cleanup()
	{
		if (m_pipe_send != nullptr)
		{
			CloseHandle(m_pipe_send);
			m_pipe_send = nullptr;
		}

		if (m_pipe_recv != nullptr)
		{
			DisconnectNamedPipe(m_pipe_recv);
			CloseHandle(m_pipe_recv);
			m_pipe_recv = nullptr;
		}

		if (m_event != nullptr)
		{
			CloseHandle(m_event);
			m_event = nullptr;
		}
	}

	bool Communicator::connect()
	{
		bool result;

		if (m_role == RoleId::kRoleClient)
		{
			clientInit();
			result = serverInit();
		}
		else
		{
			result = serverInit();
			if (result == true)
				clientInit();
		}

		return result;
	}

	void Communicator::disconnect() {
		SetEvent(m_event);
	}

	void Communicator::clientInit()
	{
		const int time_sleep = 50;
		int tries = m_client_timeout / time_sleep;

		while (1)
		{
			m_pipe_send = CreateFile(
				m_pipe_send_name.c_str(),		// pipe name 
				GENERIC_WRITE,					// Write access
				0,								// no sharing 
				NULL,							// default security attributes
				OPEN_EXISTING,					// opens existing pipe 
				0,								// default attributes 
				NULL);							// no template file 

			// Pipe has not created yet
			// Try to open it few times
			if (m_pipe_send == INVALID_HANDLE_VALUE)
			{
				Sleep(50);
				if (tries-- > 0)
					continue;
			}

			// connected, break
			if (m_pipe_send != INVALID_HANDLE_VALUE)
				break;

			// Exit if an error other than ERROR_PIPE_BUSY occurs. 
			DWORD dwErr = GetLastError();
			if (dwErr != ERROR_PIPE_BUSY)
				throw winlib::WindowsException("Error opening pipe", dwErr);

			// All pipe instances are busy, so wait for 5 seconds. 
			if (!WaitNamedPipe(m_pipe_send_name.c_str(), m_client_timeout))
				throw winlib::WindowsException("Wait pipe timeout exceeded");
		}
	}

	bool Communicator::serverInit()
	{
		OVERLAPPED wait = { 0 };
		wait.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		winlib::UniqueHandle autodel(wait.hEvent);

		m_pipe_recv = CreateNamedPipe(
			m_pipe_recv_name.c_str(), // pipe name 
			PIPE_ACCESS_INBOUND |     // read/write access 
			FILE_FLAG_OVERLAPPED,		// async
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			0,						// output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (m_pipe_recv == INVALID_HANDLE_VALUE)
			throw winlib::WindowsException("Failed to create named pipe");

		return waitForClientConnection(m_pipe_recv, &wait);
	}

	bool Communicator::waitForClientConnection(HANDLE hPipe, LPOVERLAPPED lpo)
	{
		BOOL conn_failed, fPendingIO = FALSE;
		DWORD dwErr;

		// Start an overlapped connection for this pipe instance. 
		conn_failed = ConnectNamedPipe(hPipe, lpo);
		dwErr = GetLastError();

		// Overlapped ConnectNamedPipe should return zero. 
		if (conn_failed)
			throw winlib::WindowsException("ConnectNamedPipe failed", dwErr);

		if (dwErr == ERROR_PIPE_CONNECTED)
		{
			// Already connected. Set event and exit
			SetEvent(lpo->hEvent);
			return true;
		}
		else if (dwErr == ERROR_IO_PENDING)
		{
			// Operation in progress. 
			// Wait until it finihes or exit event
			HANDLE handles[] = { lpo->hEvent, m_event };
			dwErr = WaitForMultipleObjects(RTL_NUMBER_OF(handles), 
				handles, FALSE, INFINITE);

			if (dwErr == WAIT_FAILED)
				throw winlib::WindowsException("WaitForMultipleObjects wait failed");

			if (dwErr == WAIT_OBJECT_0)
				return true;
			else
				return false;
		}
		else
		{
			// Unknown error
			throw winlib::WindowsException("ConnectNamedPipe failed", dwErr);
		}
	}

	void Communicator::putMessage(const Json& message)
	{
		BOOL ok;
		DWORD cnt_written;

		if (m_pipe_send == nullptr)
			throw winlib::WindowsException("m_pipe_send is null. Are you connected?");


		std::string data = message.dump();
		if (data.length() > BUFSIZE)
			throw winlib::WindowsException("Data length is too much");

		ok = WriteFile(
			m_pipe_send,            // pipe handle 
			data.c_str(),             // message 
			data.length(),              // message length 
			&cnt_written,             // bytes written 
			NULL);                  // not overlapped 

		if (cnt_written != data.length())
			throw winlib::WindowsException("WriteFile not full transmit");

		if (!ok)
			throw winlib::WindowsException("WriteFile failed");
	}

	Json Communicator::validateJson(Json&& message)
	{
		if (message.size() == 0)
			return Json();

		return Json(message);
	}

	Json Communicator::getMessage()
	{
		BOOL ok;
		DWORD cnt_read = 0;
		OVERLAPPED wait = { 0 };

		wait.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		winlib::UniqueHandle autodel(wait.hEvent);

		if (m_pipe_recv == nullptr)
			throw winlib::WindowsException("m_pipe_recv is null. Are you connected?");

		char buffer[BUFSIZE] = { 0 };

		ok = ReadFile(
			m_pipe_recv,           // handle to pipe 
			buffer,                // buffer to receive data 
			BUFSIZE,               // size of buffer 
			&cnt_read,             // number of bytes read 
			&wait);                // not overlapped I/O 

		if (!ok || cnt_read == 0)
		{
			DWORD dwErr = GetLastError();
			if (dwErr == ERROR_IO_PENDING)
			{
				// Operation in progress. 
				// Wait until it finihes or exit event
				HANDLE handles[] = { wait.hEvent, m_event };
				dwErr = WaitForMultipleObjects(RTL_NUMBER_OF(handles), 
					handles, FALSE, INFINITE);

				if (dwErr == WAIT_FAILED)
					throw winlib::WindowsException("WaitForMultipleObjects wait failed");

				if (dwErr == WAIT_OBJECT_0)
				{
					dwErr = GetOverlappedResult( 
								m_pipe_recv,	// handle to pipe 
								&wait,			// OVERLAPPED structure 
								&cnt_read,      // bytes transferred 
								FALSE);         // do not wait 

					if (dwErr == 0)
						throw winlib::WindowsException("WaitForMultipleObjects wait failed");

					return validateJson(Json::parse(std::string(buffer, cnt_read)));
				}
				else
					return Json{};
			}
			else if (dwErr == ERROR_BROKEN_PIPE)
				throw winlib::WindowsException("Client disconnected");
			else
				throw winlib::WindowsException("ReadFile failed", dwErr);
		}
		else
			return validateJson(Json::parse(std::string(buffer, cnt_read)));
	}
}
