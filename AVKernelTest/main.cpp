#define _CRT_SECURE_NO_WARNINGS
//#include <vld.h>
#include <windows.h>
#include <string>
#include <stdio.h>

using namespace std;

#define KPM_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PsMonitor\\ProtectedObjects" 
#define KRF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ProtectedObjects"
#define KRF_ALLOWED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\AllowedProcesses" 
#define KFF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\ProtectedObjects"
#define KFF_ALLOWED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\AllowedProcesses" 
#define KPF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PipeFilter\\ProtectedObjects"
#define KPF_ALLOWED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PipeFilter\\AllowedProcesses" 

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

DWORD DeleteRegistryValue(HKEY hBaseKey, LPCWSTR lpSubKey, LPCWSTR ValueName)
{
	DWORD Ret;
	HKEY hKey;

	Ret = RegOpenKeyEx(hBaseKey, lpSubKey, 0, KEY_WRITE, &hKey);

	if (Ret == ERROR_SUCCESS && hKey != NULL)
	{	
        Ret = RegDeleteValue(hKey, ValueName);
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

enum FilterDriverType
{
	FsFilter,
	PipeFilter,
	RegFilter
};

bool AllowAccessToObject(wstring ps_image_path, FilterDriverType type, bool cancel_allow=false)
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

    ps_image_path = GetFileKernelPath(ps_image_path);
    if (ps_image_path.length() == 0)
        return false;

    if (cancel_allow)
    {
        res = DeleteRegistryValue(HKEY_LOCAL_MACHINE,
            lpSubkey, ps_image_path.c_str());
    }
    else
    {
        res = WriteRegistryKey(HKEY_LOCAL_MACHINE,
            lpSubkey, ps_image_path.c_str(), NULL, 0, REG_SZ);
    }

	return res == ERROR_SUCCESS;
}

bool ProtectFile(wstring file_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KFF_PROTECTED_OBJECTS;
    file_path = GetFileKernelPath(file_path);

    if (file_path.length() == 0)
        return false;

    if (deprotect)
    {
        res = DeleteRegistryValue(HKEY_LOCAL_MACHINE,
            lpSubkey, file_path.c_str());
    }
    else
    {
		res = WriteRegistryKey(HKEY_LOCAL_MACHINE, lpSubkey, 
			file_path.c_str(), &access, sizeof(ACCESS_MASK), REG_DWORD);
    }

	return res == ERROR_SUCCESS;
}

bool ProtectPipe(wstring pipe_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KPF_PROTECTED_OBJECTS;
    pipe_path = GetPipeKernelPath(pipe_path);

    if (pipe_path.length() == 0)
        return false;

    if (deprotect)
    {
        res = DeleteRegistryValue(HKEY_LOCAL_MACHINE,
            lpSubkey, pipe_path.c_str());
    }
    else
    {
		res = WriteRegistryKey(HKEY_LOCAL_MACHINE, lpSubkey, 
			pipe_path.c_str(), &access, sizeof(ACCESS_MASK), REG_DWORD);
    }

	return res == ERROR_SUCCESS;
}

bool ProtectKey(HKEY base, wstring subkey_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KRF_PROTECTED_OBJECTS;
    subkey_path = GetKeyKernelPath(base, subkey_path);

    if (subkey_path.length() == 0)
        return false;

	if (deprotect)
	{
        res = DeleteRegistryValue(HKEY_LOCAL_MACHINE,
            lpSubkey, subkey_path.c_str());
	}
	else
	{
		res = WriteRegistryKey(HKEY_LOCAL_MACHINE, lpSubkey, 
			subkey_path.c_str(), &access, sizeof(ACCESS_MASK), REG_DWORD);
	}


	return res == ERROR_SUCCESS;
}

bool ProtectProcess(wstring ps_image_path, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KPM_PROTECTED_OBJECTS;
    ps_image_path = GetFileKernelPath(ps_image_path);
    
    if (ps_image_path.length() == 0)
        return false;

    if (deprotect)
    {
        res = DeleteRegistryValue(HKEY_LOCAL_MACHINE,
            lpSubkey, ps_image_path.c_str());
    }
    else
    {
		res = WriteRegistryKey(HKEY_LOCAL_MACHINE,
			lpSubkey, ps_image_path.c_str(), NULL, 0, REG_SZ);
    }

	return res == ERROR_SUCCESS;
}

bool TestOpenKey(HKEY base, const wstring& subkey_path, ACCESS_MASK access)
{
    HKEY hKey;
    DWORD Ret;
    
    Ret = RegOpenKeyEx(base, subkey_path.c_str(), 0, access, &hKey);
	if (Ret == ERROR_SUCCESS && hKey != NULL)
	{	
		RegCloseKey(hKey);
        return true;
	}

    return false;
}

bool TestOpenFile(const wstring& file_path, ACCESS_MASK access)
{
	HANDLE hPipe = CreateFile( 
			 file_path.c_str(),   // pipe name 
			 access,    // read and write access 
			 0,              // no sharing 
			 NULL,           // default security attributes
			 OPEN_EXISTING,  // opens existing pipe 
			 0,              // default attributes 
			 NULL);          // no template file 

    if (hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hPipe);
        return true;
    }

    return false;
}

bool TestOpenPipe(wstring pipe_name, ACCESS_MASK access)
{
    pipe_name = L"\\\\.\\pipe\\" + pipe_name;
    return TestOpenFile(pipe_name, access);
}

bool TestOpenProcess(DWORD pid, ACCESS_MASK access)
{
    HANDLE hProcess = OpenProcess(access, FALSE, pid);
    if (hProcess != NULL)
    {
        CloseHandle(hProcess);
        return true;
    }

    return false;
}

void usage()
{
    // protect key SOFTWARE\Microsoft 0x00020000
    // protect pipe DropboxPipe_1 0x00020000
    // protect file c:\test.exe 0x00020000
    // protect ps c:\test.exe 0x00020000

    // deprotect key SOFTWARE\Microsoft
    // deprotect pipe DropboxPipe_1
    // deprotect file c:\test.exe
    // deprotect ps c:\test.exe

    wprintf(L"Usage:\n"); 
    wprintf(L"   protect key <key-path> <access-mask-hex>\n");
    wprintf(L"   protect pipe <pipe-name> <access-mask-hex>\n");
    wprintf(L"   protect file <file-path> <access-mask-hex>\n");
    wprintf(L"   protect ps <process-path> <access-mask-hex>\n");
    wprintf(L"\n");
    wprintf(L"   deprotect key <key-path>\n");
    wprintf(L"   deprotect pipe <pipe-name>\n");
    wprintf(L"   deprotect file <file-path>\n");
    wprintf(L"   deprotect ps <process-path>\n");
    wprintf(L"\n");
    wprintf(L"   test-open key <key-path> <access-mask-hex>\n");
    wprintf(L"   test-open pipe <pipe-name> <access-mask-hex>\n");
    wprintf(L"   test-open file <file-path> <access-mask-hex>\n");
    wprintf(L"   test-open ps <pid> <access-mask-hex>\n");
    wprintf(L"\n");
    wprintf(L"   allow key <key-path> <ps-image-path>\n");
    wprintf(L"   allow pipe <pipe-name> <ps-image-path>\n");
    wprintf(L"   allow file <file-path> <ps-image-path>\n");
    wprintf(L"\n");
    wprintf(L"   deny key <key-path> <ps-image-path>\n");
    wprintf(L"   deny pipe <pipe-name> <ps-image-path>\n");
    wprintf(L"   deny file <file-path> <ps-image-path>\n");
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc == 1)
    {
        usage();
        return 0;
    }

    if (argc == 5)
    {
		DWORD access = (DWORD)wcstol(argv[4], NULL, 16);
		LPCWSTR path = argv[3];
		bool ok = false;

        if (!wcscmp(argv[1], L"protect"))
        {
			if (!wcscmp(argv[2], L"key"))
				ok = ProtectKey(HKEY_LOCAL_MACHINE, path, access);
			else if (!wcscmp(argv[2], L"file"))
				ok = ProtectFile(path, access);
			else if (!wcscmp(argv[2], L"pipe"))
				ok = ProtectPipe(path, access);
			else if (!wcscmp(argv[2], L"ps"))
				ok = ProtectProcess(path);
			else
				wprintf(L"bad args\n");
        }
        else if (!wcscmp(argv[1], L"test-open"))
        {
            if (!wcscmp(argv[2], L"key"))
                ok = TestOpenKey(HKEY_LOCAL_MACHINE, path, access);
            else if (!wcscmp(argv[2], L"file"))
                ok = TestOpenFile(path, access);
            else if (!wcscmp(argv[2], L"pipe"))
                ok = TestOpenPipe(path, access);
            else if (!wcscmp(argv[2], L"ps"))
                ok = TestOpenProcess(_wtol(argv[3]), access);
			else
				wprintf(L"bad args\n");
        }
        else if (!wcscmp(argv[1], L"allow"))
        {
            if (!wcscmp(argv[2], L"key"))
                ok = AllowAccessToObject(path, RegFilter);
			else if (!wcscmp(argv[2], L"file"))
				ok = AllowAccessToObject(path, FsFilter);
			else if (!wcscmp(argv[2], L"pipe"))
				ok = AllowAccessToObject(path, PipeFilter);
			else
				wprintf(L"bad args\n");
        }
        else if (!wcscmp(argv[1], L"deny"))
        {
            if (!wcscmp(argv[2], L"key"))
                ok = AllowAccessToObject(path, RegFilter, true);
			else if (!wcscmp(argv[2], L"file"))
				ok = AllowAccessToObject(path, FsFilter, true);
			else if (!wcscmp(argv[2], L"pipe"))
				ok = AllowAccessToObject(path, PipeFilter, true);
			else
				wprintf(L"bad args\n");
        }
        else
			wprintf(L"bad args\n");

        wprintf(L"\nResult: %s\n", ok ? L"Ok" : L"Failed");
    }
    else if (argc == 4)
    {
		LPCWSTR path = argv[3];
		bool ok = false;

        if (!wcscmp(argv[1], L"deprotect"))
        {
			if (!wcscmp(argv[2], L"key"))
				ok = ProtectKey(HKEY_LOCAL_MACHINE, path, 0, true);
			else if (!wcscmp(argv[2], L"file"))
				ok = ProtectFile(path, 0, true);
			else if (!wcscmp(argv[2], L"pipe"))
				ok = ProtectPipe(path, 0, true);
			else if (!wcscmp(argv[2], L"ps"))
				ok = ProtectProcess(path, true);
            else
                wprintf(L"bad args\n");
        }
        else
			wprintf(L"bad args\n");

        wprintf(L"\nResult: %s\n", ok ? L"Ok" : L"Failed");
    }
    else
        wprintf(L"bad args\n");

    wprintf(L"\n");
}
