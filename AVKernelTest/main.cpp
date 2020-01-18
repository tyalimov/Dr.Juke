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


DWORD WriteRegistryKey(HKEY hBaseKey, LPCWSTR lpSubKey, 
	LPCWSTR ValueName, LPCVOID Data, DWORD DataLen, DWORD DataType)
{
	DWORD Ret;
	HKEY hKey;

	Ret = RegOpenKeyEx(hBaseKey, lpSubKey, 0, KEY_WRITE, &hKey);

	if (Ret == ERROR_SUCCESS && hKey != NULL)
	{	
		Ret = RegSetValueEx(hKey, ValueName, 0, 
			DataType, (BYTE*)Data, DataLen);

		RegCloseKey(hKey);
	}

	return Ret;
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
	wstring pipe_path = L"\\\\.\\pipe\\" + pipe_name;

	HANDLE hPipe = CreateFile( 
			 pipe_path.c_str(),   // pipe name 
			 GENERIC_READ,    // read and write access 
			 0,              // no sharing 
			 NULL,           // default security attributes
			 OPEN_EXISTING,  // opens existing pipe 
			 0,              // default attributes 
			 NULL);          // no template file 

    if (hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hPipe);
        return L"\\Device\\NamedPipe\\" + pipe_name;
    }

    return wstring();
}

#define KPM_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PsMonitor\\ProtectedObjects" 

#define KRF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ProtectedObjects"
#define KRF_ALLOWED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\AllowedProcesses" 

#define KFF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\ProtectedObjects"
#define KFF_ALLOWED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\AllowedProcesses" 

#define KPF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PipeFilter\\ProtectedObjects"
#define KPF_ALLOWED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PipeFilter\\AllowedProcesses" 

enum FilterDriverType
{
	FsFilter,
	PipeFilter,
	RegFilter
};

bool AddAllowedProcess(wstring image_path, FilterDriverType type)
{
    const wchar_t* lpSubkey;
    DWORD res;

	switch (type)
	{
    case FsFilter:
		lpSubkey = KFF_ALLOWED_PROCESSES;
		break;
	case RegFilter:
		lpSubkey = KRF_ALLOWED_PROCESSES;
		break;
	case PipeFilter:
		lpSubkey = KPF_ALLOWED_PROCESSES;
		break;
	default:
		return false;
	}

    image_path = GetFileKernelPath(image_path);
    if (image_path.length() == 0)
        return false;

    res = WriteRegistryKey(HKEY_LOCAL_MACHINE, 
        lpSubkey, image_path.c_str(), NULL, 0, REG_SZ);

	return res == ERROR_SUCCESS;
}

bool AddProtectedProcess(wstring image_path)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KPM_PROTECTED_OBJECTS;
    image_path = GetFileKernelPath(image_path);
    
    if (image_path.length() == 0)
        return false;

    res = WriteRegistryKey(HKEY_LOCAL_MACHINE,
        lpSubkey, image_path.c_str(), NULL, 0, REG_SZ);

	return res == ERROR_SUCCESS;
}

bool AddProtectedFile(wstring file_path, ACCESS_MASK access)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KFF_PROTECTED_OBJECTS;
    file_path = GetFileKernelPath(file_path);

    if (file_path.length() == 0)
        return false;

    res = WriteRegistryKey(HKEY_LOCAL_MACHINE, lpSubkey, 
        file_path.c_str(), &access, sizeof(ACCESS_MASK), REG_DWORD);

	return res == ERROR_SUCCESS;
}

bool AddProtectedPipe(wstring pipe_path, ACCESS_MASK access)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KPF_PROTECTED_OBJECTS;
    pipe_path = GetPipeKernelPath(pipe_path);

    if (pipe_path.length() == 0)
        return false;

    res = WriteRegistryKey(HKEY_LOCAL_MACHINE, lpSubkey, 
        pipe_path.c_str(), &access, sizeof(ACCESS_MASK), REG_DWORD);

	return res == ERROR_SUCCESS;
}

bool AddProtectedKey(HKEY base, wstring subkey_path, ACCESS_MASK access)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KRF_PROTECTED_OBJECTS;
    subkey_path = GetKeyKernelPath(base, subkey_path);

    if (subkey_path.length() == 0)
        return false;

    res = WriteRegistryKey(HKEY_LOCAL_MACHINE, lpSubkey, 
        subkey_path.c_str(), &access, sizeof(ACCESS_MASK), REG_DWORD);

	return res == ERROR_SUCCESS;
}

int main()
{
	// GetKeyKernelPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft");
	// GetFileKernelPath(L"main.cpp");
	// GetPipeKernelPath(L"Dropboxpipe1");
    const wchar_t* p = L"c:\\Program Files (x86)\\Adobe\\Acrobat Reader DC\\Reader\\AcroBroker.exe";
    const wchar_t* f = L"c:\\Program Files (x86)\\Adobe\\Acrobat Reader DC\\Reader\\A3DUtils.dll";
    const wchar_t* k = L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\ProtectedObjects";
    AddAllowedProcess(p, FsFilter);
    AddAllowedProcess(p, PipeFilter);
    AddAllowedProcess(p, RegFilter);
    AddProtectedProcess(p);
    AddProtectedFile(f, 0x12345);
    AddProtectedKey(HKEY_LOCAL_MACHINE, k, 0x123456);
    AddProtectedPipe(L"DropboxPipe_1", 0x1234567);

	return 0;
}
