#include "utils.h"
#include "windows_exception.h"

#include <windows.h>
#include <lmcons.h>

namespace drjuke::winlib::utils
{
    [[nodiscard]] std::wstring getCurrentUserName()
    {
        wchar_t user_name[UNLEN + 1]{0};
        DWORD size = UNLEN + 1;
        
        auto status = ::GetUserNameW(user_name, &size);

        if (!status)
        {
            throw WindowsException("Can't get user name");
        }

        return std::wstring(user_name);
    }

    [[nodiscard]] SYSTEMTIME getCurrentSystemTime()
    {
        SYSTEMTIME now;
        ::GetLocalTime(&now);

        return now;
    }

    void startProcess(const std::wstring& name, std::wstring argv)
    {
        // additional information
        STARTUPINFOW        startup_info;
        PROCESS_INFORMATION process_information;

        // set the size of the structures
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.cb = sizeof(startup_info);
        ZeroMemory(&process_information, sizeof(process_information));

        // start the program up
        auto status = CreateProcessW
        (
            name.c_str(),        // The path
            argv.data(),         // Command line
            nullptr,             // Process handle not inheritable
            nullptr,             // Thread handle not inheritable
            FALSE,               // Set handle inheritance to FALSE
            CREATE_NEW_CONSOLE,  // Opens file in a separate console
            nullptr,             // Use parent's environment block
            nullptr,             // Use parent's starting directory 
            &startup_info,       // Pointer to STARTUPINFO structure
            &process_information // Pointer to PROCESS_INFORMATION structure
        );

        if (!status)
            throw WindowsException("Can't start process");

        // Wait for process to finish
        WaitForSingleObject(process_information.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(process_information.hProcess);
        CloseHandle(process_information.hThread);
    }

    void startService(const std::wstring &name)
    {

    }
}
