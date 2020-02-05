#include "utils.h"
#include "windows_exception.h"
#include "raii.h"

#include <windows.h>
#include <lmcons.h>


namespace drjuke::winlib::utils
{
	typedef LONG NTSTATUS;

	#ifndef STATUS_SUCCESS
	#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
	#endif

	#ifndef STATUS_BUFFER_TOO_SMALL
	#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023L)
	#endif

	typedef enum _KEY_INFORMATION_CLASS {
	  KeyBasicInformation,
	  KeyNodeInformation,
	  KeyFullInformation,
	  KeyNameInformation,
	  KeyCachedInformation,
	  KeyFlagsInformation,
	  KeyVirtualizationInformation,
	  KeyHandleTagsInformation,
	  KeyTrustInformation,
	  KeyLayerInformation,
	  MaxKeyInfoClass
	} KEY_INFORMATION_CLASS;

	typedef struct _KEY_NAME_INFORMATION {
	  ULONG NameLength;
	  WCHAR Name[1];
	} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

	typedef DWORD (__stdcall *NtQueryKeyFunc)(
		HANDLE  KeyHandle,
		int KeyInformationClass,
		PVOID  KeyInformation,
		ULONG  Length,
		PULONG  ResultLength);


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

	std::wstring getFileKernelPath(const std::wstring& file_path)
	{
		DWORD length;
		TCHAR path[MAX_PATH];
		std::wstring result;

		HANDLE hFile = CreateFileW(file_path.c_str(),
			0,
			0,  
			0,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			0);
		
		if (INVALID_HANDLE_VALUE != hFile)
		{
			length = GetFinalPathNameByHandle(
				hFile, path, MAX_PATH, VOLUME_NAME_NT);

			CloseHandle(hFile);

			// length indicates returned path length
			// or error status on failure
			if (length > 0)
				result = std::wstring(path, path + length);
			else
				throw WindowsException("GetFinalPathNameByHandle failed", length);

		}
		else
			throw WindowsException("CreateFileW failed");

		return result;
	}

	std::wstring getKeyKernelPath(HKEY hBaseKey, const std::wstring& subKey)
	{
		DWORD size = 0;
		DWORD result = 0;
		HKEY hKey = NULL;
		std::wstring keyPath;

		result = RegOpenKey(hBaseKey, subKey.c_str(), &hKey);
		if (result == ERROR_SUCCESS && hKey != NULL)
		{
			UniqueRegHandle cleaner(hKey);

			HMODULE ntdll = LoadLibrary(L"ntdll.dll");
			if (ntdll != NULL) 
			{
				NtQueryKeyFunc NtQueryKey = (NtQueryKeyFunc)GetProcAddress(ntdll, "NtQueryKey");

				if (NtQueryKey != NULL) 
				{
					result = NtQueryKey(hKey, KeyNameInformation, 0, 0, &size);
					if (result == STATUS_BUFFER_TOO_SMALL)
					{
						size = size + 2;
						auto buffer = std::make_unique<wchar_t[]>(size / sizeof(wchar_t));
						KEY_NAME_INFORMATION* info = (KEY_NAME_INFORMATION*)buffer.get();

						result = NtQueryKey(hKey, KeyNameInformation, buffer.get(), size, &size);
						if (result == STATUS_SUCCESS)
							keyPath = std::wstring(info->Name, info->NameLength / sizeof(wchar_t));
						else
							throw WindowsException("NtQueryKey failed", result);
					}
					else
						throw WindowsException("Unexpected NtQueryKey return status", result);
				}
				else
					throw WindowsException("Get NtQueryKey address failed");
			}
			else
				throw WindowsException("Load ntdll failed");
		}
		else
			throw WindowsException("RegOpenKey failed", result);

		return keyPath;
	}
}
