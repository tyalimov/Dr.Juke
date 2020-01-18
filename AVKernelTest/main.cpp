#define _CRT_SECURE_NO_WARNINGS
#include <vld.h>
#include <windows.h>
#include <string>

using namespace std;

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

BOOL CreateRegistryKey(HKEY hKeyParent, PWCHAR subkey)
{
	DWORD dwDisposition; //It verify new key is created or open existing key
	HKEY  hKey;
	DWORD Ret;

	Ret = RegCreateKeyEx(
		hKeyParent,
		subkey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition);

	if (Ret != ERROR_SUCCESS)
	{
		if (Ret == ERROR_ACCESS_DENIED)
			printf("Error opening or creating new key: Access denied\n");
		else
			printf("Error opening or creating new key: Unknown error 0x%08X\n", Ret);

		return FALSE;
	}

	RegCloseKey(hKey);
	printf("Succesfully opened or created new key\n");
	return TRUE;
}

BOOL WriteRegistryKey(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD data)
{
	DWORD Ret; //use to check status
	HKEY hKey; //key

	Ret = RegOpenKeyEx(
		hKeyParent,
		subkey,
		0,
		KEY_WRITE,
		&hKey
		);

	if (Ret == ERROR_SUCCESS)
	{	
		DWORD status = RegSetValueEx(hKey, valueName, 0, 
			REG_DWORD, reinterpret_cast<BYTE*>(&data), sizeof(data));

		if (status != ERROR_SUCCESS)
		{
			printf("Error writing key: 0x%08X\n", status);
			RegCloseKey(hKey);
			return FALSE;
		}

		//close the key
		RegCloseKey(hKey);
		printf("Successfull write to key\n");
		return TRUE;
	}
	else if (Ret == ERROR_ACCESS_DENIED)
		printf("Error opening or creating new key: Access denied\n");
	else
		printf("Error opening or creating new key: Unknown error 0x%08X\n", Ret);

	return FALSE;
}

wstring GetFileKernelPath(const wstring& file_path)
{
    wstring result;
    TCHAR path[MAX_PATH];

    HANDLE hFile = CreateFile(file_path.c_str(),
        0,
        0,  
        0,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0);
    
    if (INVALID_HANDLE_VALUE != hFile)
    {
        auto rcode = GetFinalPathNameByHandle(
            hFile, path, MAX_PATH, VOLUME_NAME_NT);

        if (rcode)
            result = wstring(path, path + rcode);

		CloseHandle(hFile);
    }

    return result;
}

wstring GetKeyKernelPath(HKEY hBaseKey, const wstring& lpSubKey)
{
    DWORD res;
    HKEY hKey;
    wstring keyPath;

    res = RegOpenKey(hBaseKey, lpSubKey.c_str(), &hKey);
    if (res == ERROR_SUCCESS && hKey != NULL)
    {
        HMODULE ntdll = LoadLibrary(L"ntdll.dll");
        if (ntdll != NULL) {

            NtQueryKeyFunc NtQueryKey = (NtQueryKeyFunc)GetProcAddress(ntdll, "NtQueryKey");

            if (NtQueryKey != NULL) {
                DWORD size = 0;
                DWORD result = 0;
                result = NtQueryKey(hKey, KeyNameInformation, 0, 0, &size);
                if (result == STATUS_BUFFER_TOO_SMALL)
                {
                    size = size + 2;
                    wchar_t* buffer = new wchar_t[size / sizeof(wchar_t)]; // size is in bytes
                    KEY_NAME_INFORMATION* info = (KEY_NAME_INFORMATION*)buffer;

					result = NtQueryKey(hKey, KeyNameInformation, buffer, size, &size);
					if (result == STATUS_SUCCESS)
						keyPath = wstring(info->Name, info->NameLength / sizeof(wchar_t));

					delete[] buffer;
                }
            }
        }
    }
    return keyPath;
}

wstring GetPipeKernelPath(const wstring& pipe_name)
{
	return L"\\Device\\NamedPipe\\" + pipe_name;
}

bool AddProtectedProcess(const wstring& image_path, ACCESS_MASK access)
{

}

bool AddProtectedFile(const wstring& image_path, ACCESS_MASK access)
{

}

bool AddProtectedPipe(const wstring& image_path, ACCESS_MASK access)
{

}

int main()
{
	GetKeyKernelPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft");
	GetFileKernelPath(L"main.cpp");
	GetPipeKernelPath(L"Dropboxpipe1");

	return 0;
}
