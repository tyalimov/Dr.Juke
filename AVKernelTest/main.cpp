#define _CRT_SECURE_NO_WARNINGS
//#include <vld.h>
#include <windows.h>
#include <string>
#include <stdio.h>

using namespace std;

#define DR_JUKE_BASE_KEY L"SOFTWARE\\Dr.Juke"
#define DR_JUKE_SEC_KEY L"SOFTWARE\\Dr.Juke\\AVSecGeneric"
#define DR_JUKE_TEST_KEY L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\Test"


const wchar_t* keys[] = {
    L"AVSecGeneric\\PsMonitor\\ProtectedObjects",
    L"AVSecGeneric\\PsMonitor\\ExcludedProcesses",
    L"AVSecGeneric\\RegFilter\\ProtectedObjects",
    L"AVSecGeneric\\RegFilter\\ExcludedProcesses",
    L"AVSecGeneric\\FsFilter\\ProtectedObjects",
    L"AVSecGeneric\\FsFilter\\ExcludedProcesses",
    L"AVSecGeneric\\PipeFilter\\ProtectedObjects",
    L"AVSecGeneric\\PipeFilter\\ExcludedProcesses",
};

#define KPM_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PsMonitor\\ProtectedObjects" 
#define KPM_EXCLUDED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\PsMonitor\\ExcludedProcesses"

#define KRF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ProtectedObjects"
#define KRF_EXCLUDED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ExcludedProcesses" 

#define KFF_PROTECTED_OBJECTS L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\ProtectedObjects"
#define KFF_EXCLUDED_PROCESSES L"SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter\\ExcludedProcesses" 

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

#pragma region api

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
	RegFilter,
    PsMonitor,
};

DWORD AddRemoveExcludedProcess(wstring ps_image_path, FilterDriverType type, bool remove=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

	switch (type)
	{
    case FsFilter:
	case PipeFilter:
		lpSubkey = KFF_EXCLUDED_PROCESSES;
		break;
	case RegFilter:
		lpSubkey = KRF_EXCLUDED_PROCESSES;
		break;
    case PsMonitor:
		lpSubkey = KPM_EXCLUDED_PROCESSES;
		break;
	default:
		return ERROR_UNSUPPORTED_TYPE;
	}

    ps_image_path = GetFileKernelPath(ps_image_path);
    if (ps_image_path.length() == 0)
        return ERROR_FILE_NOT_FOUND;

    if (remove)
    {
        res = DeleteRegistryValue(HKEY_LOCAL_MACHINE,
            lpSubkey, ps_image_path.c_str());
    }
    else
    {
        res = WriteRegistryKey(HKEY_LOCAL_MACHINE,
            lpSubkey, ps_image_path.c_str(), NULL, 0, REG_SZ);
    }

	return res;
}

DWORD ProtectFile(wstring file_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KFF_PROTECTED_OBJECTS;
    file_path = GetFileKernelPath(file_path);

    if (file_path.length() == 0)
        return ERROR_FILE_NOT_FOUND;

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

	return res;
}

DWORD ProtectPipe(wstring pipe_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KFF_PROTECTED_OBJECTS;
    pipe_path = GetPipeKernelPath(pipe_path);

    if (pipe_path.length() == 0)
        return ERROR_FILE_NOT_FOUND;

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

	return res;
}

DWORD ProtectKey(HKEY base, wstring subkey_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    subkey_path = GetKeyKernelPath(base, subkey_path);
    if (subkey_path.length() == 0)
        return ERROR_FILE_NOT_FOUND;

    lpSubkey = KRF_PROTECTED_OBJECTS;
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


	return res;
}

DWORD ProtectProcess(wstring ps_image_path, ACCESS_MASK access=0, bool deprotect=false)
{
    const wchar_t* lpSubkey;
    DWORD res;

    lpSubkey = KPM_PROTECTED_OBJECTS;
    ps_image_path = GetFileKernelPath(ps_image_path);
    
    if (ps_image_path.length() == 0)
        return ERROR_FILE_NOT_FOUND;

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

	return res;
}

#pragma endregion api

#pragma region test_primitives

DWORD TestOpenKey(HKEY base, const wstring& subkey_path, ACCESS_MASK access)
{
    HKEY hKey;
    DWORD Ret;
    
    Ret = RegOpenKeyEx(base, subkey_path.c_str(), 0, access, &hKey);
	if (Ret == ERROR_SUCCESS && hKey != NULL)
		RegCloseKey(hKey);

    return Ret;
}

DWORD TestOpenFile(const wstring& file_path, ACCESS_MASK access)
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
        CloseHandle(hPipe);

    return GetLastError();
}

DWORD TestOpenPipe(wstring pipe_name, ACCESS_MASK access)
{
    pipe_name = L"\\\\.\\pipe\\" + pipe_name;
    return TestOpenFile(pipe_name, access);
}

DWORD TestOpenProcess(DWORD pid, ACCESS_MASK access)
{
    HANDLE hProcess = OpenProcess(access, FALSE, pid);
    if (hProcess != NULL)
        CloseHandle(hProcess);

    return GetLastError();
}

#pragma endregion test_primitives

#pragma region tests

bool TestBadValue()
{
    DWORD Data = 0x12345678;
    HKEY hBase = HKEY_LOCAL_MACHINE;
    const wchar_t* ValueName1 = L"bla-bla";
    const wchar_t* ValueName2 = L"bla-bla2";

    // Bad reg dword value
    WriteRegistryKey(HKEY_LOCAL_MACHINE, KRF_PROTECTED_OBJECTS, 
        ValueName1, &Data, sizeof(DWORD), REG_DWORD);

    // Bad value type 
    WriteRegistryKey(HKEY_LOCAL_MACHINE, KRF_PROTECTED_OBJECTS, 
        ValueName2, 0, 0, REG_SZ);

    DeleteRegistryValue(HKEY_LOCAL_MACHINE, KRF_PROTECTED_OBJECTS, ValueName1);
    DeleteRegistryValue(HKEY_LOCAL_MACHINE, KRF_PROTECTED_OBJECTS, ValueName2);

    return true;
}

void resetRegistryKeys()
{
    HKEY hKey;
    HKEY hOut;
    DWORD err;
    
    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
            DR_JUKE_BASE_KEY, 0, 0, 0, KEY_ALL_ACCESS, NULL, &hKey, 0);

    if (err == ERROR_SUCCESS)
    {
        for (int i = 0; i < RTL_NUMBER_OF(keys); i++)
        {
            err = RegDeleteKey(hKey, keys[i]);
        }

        CloseHandle(hKey);
    }

    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
            DR_JUKE_BASE_KEY, 0, 0, 0, KEY_ALL_ACCESS, NULL, &hKey, 0);

    if (err == ERROR_SUCCESS)
    {
		for (int i = 0; i < RTL_NUMBER_OF(keys); i++)
		{
		    err = RegCreateKey(hKey, keys[i], &hOut);
            CloseHandle(hOut);
		}

        CloseHandle(hKey);
    }

}

void TestExistingPid(const wchar_t* self)
{
    HKEY hKey;
    DWORD err;

    resetRegistryKeys();

	err = RegCreateKey(HKEY_LOCAL_MACHINE, DR_JUKE_TEST_KEY, &hKey);

    ProtectKey(HKEY_LOCAL_MACHINE, DR_JUKE_TEST_KEY, 0);

    err = TestOpenKey(HKEY_LOCAL_MACHINE, DR_JUKE_TEST_KEY, KEY_READ);
    printf("TestOpenKey open res=%d\n", err);
	
    AddRemoveExcludedProcess(self, RegFilter);

    err = TestOpenKey(HKEY_LOCAL_MACHINE, DR_JUKE_TEST_KEY, KEY_READ);
    printf("TestOpenKey open after exclude res=%d\n", err);

    err = RegDeleteKey(HKEY_LOCAL_MACHINE, DR_JUKE_TEST_KEY);

    CloseHandle(hKey);
    resetRegistryKeys();
}

#pragma endregion tests

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
    wprintf(L"   flt-bypass key <ps-image-path>\n");
    wprintf(L"   flt-bypass pipe <ps-image-path>\n");
    wprintf(L"   flt-bypass file <ps-image-path>\n");
    wprintf(L"   flt-bypass ps <ps-image-path>\n");
    wprintf(L"\n");
    wprintf(L"   flt-no-bypass key <key-path> <ps-image-path>\n");
    wprintf(L"   flt-no-bypass pipe <pipe-name> <ps-image-path>\n");
    wprintf(L"   flt-no-bypass file <file-path> <ps-image-path>\n");
    wprintf(L"   flt-no-bypass ps <file-path> <ps-image-path>\n");
}

class HexDeserializer
{
	using u8 = unsigned char;
    char* mBuffer = nullptr;
    ULONG mLength = 0;

    inline char substitute(u8 i) {
        return (i <= 9 ? '0' + i : 'A' - 10 + i);
    }

    void int2hex(u8 in, char* out)
    {
		out[0] = substitute((in & 0xF0) >> 4);   
		out[1] = substitute(in & 0x0F);
    }

public:

    HexDeserializer(const char* buffer, ULONG length) 
    {
        mLength = length * 2;
		mBuffer = new char[mLength + 1];

		if (mBuffer)
		{
			for (ULONG i = 0, j = 0; i < length; i++, j += 2)
				int2hex(buffer[i], &mBuffer[j]);

			mBuffer[mLength] = '\0';
		}
    }

    ~HexDeserializer()
    {
        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
        }
    }

    PCHAR getHexText() {
        return mBuffer;
    }

    ULONG getLength() {
        return mLength;
    }
};

class HexSerializer
{
	using u8 = unsigned char;
    char* mBuffer = nullptr;
    ULONG mLength = 0;

    inline char substitute(u8 i) {
        return (i <= '9' ? i - '0' : i + 10 - 'A');
    }

    u8 hex2int(const char* in)
    {
        return (substitute(in[0]) << 4)
            + substitute(in[1]);
    }

public:

    HexSerializer(const char* buffer, ULONG length)
    {
        mLength = length / 2;
		mBuffer = new char[mLength + 1];
		
        if (mBuffer)
		{
			for (ULONG i = 0, j = 0; i < mLength; i++, j += 2)
				mBuffer[i] = hex2int(&buffer[j]);

			mBuffer[mLength] = '\0';
		}
    }

    ~HexSerializer()
    {
        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
        }
    }

    PCHAR getBinaryData() {
        return mBuffer;
    }

    ULONG getLength() {
        return mLength;
    }
};

#define TCP 1
#define UDP 2
#define IPV4 4
#define ICMPV4 3

#include <list>

class NetFilterRule
{
    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

    bool m_allow = true;
    u8 m_protocol = 0;
    u16 m_priority = 0;
    
    u16 m_local_port = 0;
    u16 m_remote_port = 0;

    u32 m_local_ip = 0;
    u32 m_remote_ip = 0;

    u16 m_offset = 0;
    string m_content;

    static const u16 mc_port_any = 0;
    static const u16 mc_max_offset = (u16)-1;

public:

    NetFilterRule() = default;
    ~NetFilterRule() = default;

    NetFilterRule(const NetFilterRule& other) = default;

    bool isAllowPerm() {
        return m_allow;
    }

    bool isProtocolMatch(u8 protocol) const {
        return m_protocol == protocol; // || m_protocol == IP
    }

    bool isLocalIpMatch(u32 ip) const {
        return m_local_ip == ip;
    }

    bool isRemoteIpMatch(u32 ip) const {
        return m_remote_ip == ip;
    }

    bool isLocalPortMatch(u32 port) const
    {
        return m_local_port == port
            || m_local_port == mc_port_any;
    }

    bool isRemotePortMatch(u32 port) const
    {
        return m_remote_port == port
            || m_remote_port == mc_port_any;
    }

    bool isContentMatch(const char* buffer, size_t length) const
    {
        if (!m_content.length() == 0)
            return true;

        size_t content_length = m_content.length();
        if (length > m_offset)
        {
            if (length - m_offset > content_length)
            {
                return !m_content.compare(m_offset,
                    content_length, buffer);
            }
        }

        return false;
    }

    void setAllowPerm(bool allow) {
        m_allow = allow;
    }

    void setProtocol(u8 protocol) {
        m_protocol = protocol;
    }

    void setLocalIp(u32 local_ip) {
        m_local_ip = local_ip;
    }

    void setRemoteIp(u32 remote_ip) {
        m_remote_ip = remote_ip;
    }

    void setLocalPort(u16 local_port) {
        m_local_port = local_port;
    }

    void setRemotePort(u16 remote_port) {
        m_remote_port = remote_port;
    }

    void setContent(const char* content, size_t len, u16 offset) 
    {
        m_content = string(content, len);
        m_offset = offset;
    }

    void setPriority(u16 priority) {
        m_priority = priority;
    }

    u16 getPriority() {
        return m_priority;
    }

};

class NetFilterRuleParser
{
	// deny tcp 0.0.0.0:12 0.0.0.0:13 priority=2 offset=13 |00123456|
	// [allow/deny] õtcp/udp/icmp] [any/0.0.0.0]:[12/any] 0.0.0.0:13 [<offset=13|00aavvvsss|>]

    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

public:

	u8 wchar2dec(wchar_t c)
	{
        if (c >= '\u0030' && c <= '\u0039')
            return c - '\u0030';

        return 0xFF;
	}

	list<wstring> split(const wstring& s, wchar_t spl)
	{
		size_t j = 0, i = 0;
		list<wstring> chunks;
		wstring sub;

		for (j = s.find(spl, 0); j != wstring::npos; j = s.find(spl, j + 1))
		{
			sub = s.substr(i, j - i);
			chunks.push_back(move(sub));
			i = j + 1;
		}

		sub = s.substr(i, s.length() - i);
		chunks.push_back(move(sub));

		return chunks;
	}

    // EF FF FF FA -> 239.255.255.250
    int parse_ip(const wstring& ip_string, u32* ip)
    {
        int i = 24;
        *ip = 0;

        auto chunks = split(ip_string, L'.');
        if (chunks.size() != 4)
            return -1;

        u8 d1, d2, d3, octet;
        for (const auto& ip_octet_s: chunks)
        {
            auto length = ip_octet_s.length();
            if (length == 1)
            {
                octet = wchar2dec(ip_octet_s[0]);
                if (octet == 0xFF)
                    return -1;
            }
            else if (length == 2)
            {
                d1 = wchar2dec(ip_octet_s[0]);
                d2 = wchar2dec(ip_octet_s[1]);

                if (d1 == 0xFF || d2 == 0xFF)
                    return -1;

                if ((int)d1 * 10 + (int)d2 > 0xFF)
                    return -1;

                octet = d1 * 10 + d2;
            }
            else if (length == 3)
            {
                d1 = wchar2dec(ip_octet_s[0]);
                d2 = wchar2dec(ip_octet_s[1]);
                d3 = wchar2dec(ip_octet_s[2]);

                if (d1 == 0xFF || d2 == 0xFF || d3 == 0xFF)
                    return -1;

                if ((int)d1 * 100 + (int)d2 * 10 + (int)d3 > 0xFF)
                    return -1;
                
                octet = d1 * 100 + d2 * 10 + d3;
            }
            else
                return -1;

            *ip = *ip + (octet << i);
            i -= 8;
        }

        return 0;
    }

    int parse_ip_port(const wstring& ip_port, u32* ip, u16* port)
    {
        auto chunks = split(ip_port, L':');
        if (chunks.size() != 2)
            return -1;

        wstring ip_str = chunks.front();
        if (ip_str.length() > 0)
        {
            if (ip_str == L"any")
                *ip = 0;
            else
            {
                if (parse_ip(ip_str, ip) == -1)
                    return -1;
            }
        }
        else
            return -1;

        chunks.pop_front();
        wstring port_str = chunks.front();

        if (port_str.length() > 0)
        {
            if (port_str == L"any")
                *port = 0;
            else
            {
                int p = _wtoi(port_str.c_str());

                if (p > 0 && p < 65536)
                    *port = (u16)p;
                else
                    return -1;
            }
        }

        return 0;
    }

    int parse_eq_str(const wstring& pri_str, const wstring& name, u16* out)
    {
        auto chunks = split(pri_str, '=');
        if (chunks.size() != 2)
            return -1;
        else
        {
            wstring value = chunks.front();
            if (value != name)
                return -1;
            else
            {
                chunks.pop_front();
                value = chunks.front();

                int x = _wtoi(value.c_str());
                if (x > 0 && x <= (u16)-1)
                    *out = (u16)x;
                else
                    return -1;

            }
        }

        return 0;
    }

	int parse(const wstring& rule, NetFilterRule* ruleObj)
	{
        wstring value;
        u32 ip;
        u16 port, priority;
        size_t size;
        int res;

        auto chunks = split(rule, L' ');
        size = chunks.size();

        if (size != 5 && size != 7)
            return -1;

        // 1. permission allow / deny
        value = chunks.front();
        if (value == L"allow")
            ruleObj->setAllowPerm(true);
        else if (value == L"deny")
            ruleObj->setAllowPerm(false);
        else
            return -1;

        // 2. protocol tcp/udp/icmp/ip
        chunks.pop_front();
        value = chunks.front();
        if (value == L"tcp")
            ruleObj->setProtocol(TCP);
        else if (value == L"udp")
            ruleObj->setProtocol(UDP);
        else if (value == L"icmp")
            ruleObj->setProtocol(ICMPV4);
        else if (value == L"ip")
            ruleObj->setProtocol(IPV4);
        else
            return -1;

        // 3 ip:port from
        chunks.pop_front();
        value = chunks.front();
        
        res = parse_ip_port(value, &ip, &port);
        if (res)
            return -1;
        else
        {
            ruleObj->setLocalIp(ip);
            ruleObj->setLocalPort(port);
        }

        // 4 ip:port to
        chunks.pop_front();
        value = chunks.front();

        res = parse_ip_port(value, &ip, &port);
        if (res)
            return -1;
        else
        {
            ruleObj->setRemoteIp(ip);
            ruleObj->setRemotePort(port);
        }

        // 5 priority
        chunks.pop_front();
        value = chunks.front();
        res = parse_eq_str(value, L"priority", &priority);
        if (res)
            return -1;
        else
            ruleObj->setPriority(priority);

        if (size == 7)
        {
            // 6 offset
            u16 offset;
            wstring ws_content;
            string content;

			chunks.pop_front();
			value = chunks.front();
			res = parse_eq_str(value, L"offset", &offset);
			if (res)
				return -1;

            // 7 content |001100|
            chunks.pop_front();
            value = chunks.front();

            if (value.length() >= 4)
            {
                if (value.front() == L'|' && value.back() == L'|')
                    ws_content = wstring(++value.begin(), --value.end());

                string content(ws_content.begin(), ws_content.end());

                if (content.length() % 2 != 0)
                    return -1;

                bool ok;
                for (const char& x : content)
                {
                    ok = (x >= '0' && x <= '9' 
                        || x >= 'A' && x <= 'F' 
                        || x >= 'a' && x <= 'f');

                    if (!ok)
                        return -1;
                }

                HexSerializer serializer(content.c_str(), (ULONG)content.length());

                ruleObj->setContent(serializer.getBinaryData(),
                    serializer.getLength(), offset);
            }
            else
                return -1;
        }

        return 0;
	}
};



int wmain(int argc, wchar_t *argv[])
{
    //const wchar_t* r = L"deny tcp 239.255.255.250:12 123.11.1.0:13 priority=2 offset=13 |0501025403|";
    const wchar_t* r = L"deny tcp 255.255.255.250:65535 123.11.1.0:13 priority=7800 offset=65535 |0123456789ABCDEFabcdef|";

    NetFilterRuleParser parser;
    NetFilterRule rule;
    int res = parser.parse(r, &rule);
    printf("res = %d\n", res);

    //TestExistingPid(argv[0]);

    if (true)
        return 1;

    if (argc == 1)
    {
        usage();
        return 0;
    }

    if (argc == 5)
    {
		LPCWSTR path = argv[3];
		DWORD res = ERROR_BAD_ARGUMENTS;
		DWORD access = (DWORD)wcstol(argv[4], NULL, 16);

        if (!wcscmp(argv[1], L"protect"))
        {
			if (!wcscmp(argv[2], L"key"))
				res = ProtectKey(HKEY_LOCAL_MACHINE, path, access);
			else if (!wcscmp(argv[2], L"file"))
				res = ProtectFile(path, access);
			else if (!wcscmp(argv[2], L"pipe"))
				res = ProtectPipe(path, access);
			else if (!wcscmp(argv[2], L"ps"))
				res = ProtectProcess(path, access);
			else
				wprintf(L"bad args\n");
        }
        else if (!wcscmp(argv[1], L"test-open"))
        {
            if (!wcscmp(argv[2], L"key"))
                res = TestOpenKey(HKEY_LOCAL_MACHINE, path, access);
            else if (!wcscmp(argv[2], L"file"))
                res = TestOpenFile(path, access);
            else if (!wcscmp(argv[2], L"pipe"))
                res = TestOpenPipe(path, access);
            else if (!wcscmp(argv[2], L"ps"))
                res = TestOpenProcess(_wtol(argv[3]), access);
			else
				wprintf(L"bad args\n");
        }
        else
			wprintf(L"bad args\n");

        if (res == ERROR_SUCCESS)
            wprintf(L"\nResult: Ok\n");
        else
            wprintf(L"\nResult: failed - 0x%08X\n", res);

    }
    else if (argc == 4)
    {
		LPCWSTR path = argv[3];
		DWORD res = ERROR_BAD_ARGUMENTS;

        if (!wcscmp(argv[1], L"deprotect"))
        {
			if (!wcscmp(argv[2], L"key"))
				res = ProtectKey(HKEY_LOCAL_MACHINE, path, 0, true);
			else if (!wcscmp(argv[2], L"file"))
				res = ProtectFile(path, 0, true);
			else if (!wcscmp(argv[2], L"pipe"))
				res = ProtectPipe(path, 0, true);
			else if (!wcscmp(argv[2], L"ps"))
				res = ProtectProcess(path, 0, true);
            else
                wprintf(L"bad args\n");
        }
        else if (!wcscmp(argv[1], L"flt-bypass"))
        {
            if (!wcscmp(argv[2], L"key"))
                res = AddRemoveExcludedProcess(path, RegFilter);
			else if (!wcscmp(argv[2], L"file"))
				res = AddRemoveExcludedProcess(path, FsFilter);
			else if (!wcscmp(argv[2], L"pipe"))
				res = AddRemoveExcludedProcess(path, PipeFilter);
			else if (!wcscmp(argv[2], L"ps"))
				res = AddRemoveExcludedProcess(path, PsMonitor);
			else
				wprintf(L"bad args\n");
        }
        else if (!wcscmp(argv[1], L"flt-no-bypass"))
        {
            if (!wcscmp(argv[2], L"key"))
                res = AddRemoveExcludedProcess(path, RegFilter, true);
			else if (!wcscmp(argv[2], L"file"))
				res = AddRemoveExcludedProcess(path, FsFilter, true);
			else if (!wcscmp(argv[2], L"pipe"))
				res = AddRemoveExcludedProcess(path, PipeFilter, true);
			else if (!wcscmp(argv[2], L"ps"))
				res = AddRemoveExcludedProcess(path, PsMonitor, true);
			else
				wprintf(L"bad args\n");
        }
        else
			wprintf(L"bad args\n");

        if (res == ERROR_SUCCESS)
            wprintf(L"\nResult: Ok\n");
        else
            wprintf(L"\nResult: failed - 0x%08X\n", res);
    }
    else
        wprintf(L"bad args\n");

    wprintf(L"\n");
}
