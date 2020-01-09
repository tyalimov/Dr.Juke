#include "util.h"
#include "common.h"
#include <EASTL/utility.h>

using namespace eastl;

VOID GuardedMutex::acquire() {
	KeAcquireGuardedMutex(&_mutex);
}

VOID GuardedMutex::release() {
	KeReleaseGuardedMutex(&_mutex);
}

BOOLEAN GuardedMutex::try_acquire() {
	return KeTryToAcquireGuardedMutex(&_mutex);
}

GuardedMutex::GuardedMutex() {
	KeInitializeGuardedMutex(&_mutex);
}

BOOLEAN IsWin8OrGreater()
{
    RTL_OSVERSIONINFOEXW VersionInfo = {0};
    ULONGLONG ConditionMask = 0;
    NTSTATUS Status;
    BOOLEAN Result;

    //
    // Set VersionInfo to Win7's version number and then use
    // RtlVerifVersionInfo to see if this is win8 or greater.
    //
    
    VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
    VersionInfo.dwMajorVersion = 6;
    VersionInfo.dwMinorVersion = 1;

    VER_SET_CONDITION(ConditionMask, VER_MAJORVERSION, VER_LESS_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_MINORVERSION, VER_LESS_EQUAL);

    Status = RtlVerifyVersionInfo(&VersionInfo,
                                  VER_MAJORVERSION | VER_MINORVERSION,
                                  ConditionMask);
    if (NT_SUCCESS(Status)) 
    {
        kprintf(TRACE_OS_VER, "This machine is running Windows 7 or an older OS");
        Result = FALSE;
    } 
    else if (Status == STATUS_REVISION_MISMATCH) 
    {
        kprintf(TRACE_OS_VER, "This machine is running Windows 8 or a newer OS");
        Result = TRUE;
    } 
    else 
    {
        kprintf(TRACE_OS_VER, "RtlVerifyVersionInfo returned unexpected status 0x%08X", Status);
        Result = FALSE;  
    } 

    return Result;
}

wstring GetKeyNameByHandle(HANDLE KeyHandle)
{
	wstring KeyName = L"";
	NTSTATUS Status;
	ULONG DataSize;
	CHAR* InfoCh;

	Status = ZwQueryKey(KeyHandle, KeyBasicInformation, NULL, 0, &DataSize);
	if (Status == STATUS_BUFFER_TOO_SMALL || Status == STATUS_BUFFER_OVERFLOW)
	{
		if (DataSize > 0)
		{
			InfoCh = new CHAR[DataSize];
			if (InfoCh != nullptr)
			{
				PKEY_BASIC_INFORMATION Info = (PKEY_BASIC_INFORMATION)InfoCh;

				Status = ZwQueryKey(KeyHandle, KeyBasicInformation, Info, DataSize, &DataSize);
				if (NT_SUCCESS(Status))
					KeyName = wstring(Info->Name, Info->NameLength / sizeof(WCHAR));
			}
		}
	}

	return KeyName;
}

namespace str_util
{
	bool startsWith(const wstring& str, const wstring& substr) 
	{
		auto n = substr.length();
		return (str.length() >= n) &&
			(wcsncmp(str.c_str(), substr.c_str(), n) == 0);
	}

	bool startsWith(const string& str, const string& substr) 
	{
		auto n = substr.length();
		return (str.length() >= n) && 
			(strncmp(str.c_str(), substr.c_str(), n) == 0);
	}

	bool startsWith(const wstring& str, const wchar_t* substr) 
	{
		auto n = wcslen(substr);
		return (str.length() >= n) &&
			(wcsncmp(str.c_str(), substr, n) == 0);
	}

	bool startsWith(const string& str, const char* substr) {
		auto n = strlen(substr);
		return (str.length() >= n) &&
			(strncmp(str.c_str(), substr, n) == 0);
	}

	bool startsWith(const wstring* str, const wchar_t* substr) {
		auto n = wcslen(substr);
		return (str->length() >= n) &&
			(wcsncmp(str->c_str(), substr, n) == 0);
	}

	bool startsWith(const string* str, const char* substr) {
		auto n = strlen(substr);
		return (str->length() >= n) &&
			(strncmp(str->c_str(), substr, n) == 0);
	}

	void makeLower(wstring* str)
	{
		auto n = str->size();
		for (auto i = 0; i < n; i++)
			str->at(i) = towlower(str->at(i));
	}

	void makeLower(string* str)
	{
		auto n = str->size();
		for (auto i = 0; i < n; i++)
			str->at(i) = (char)tolower(str->at(i));
	}

	bool compareIns(string str1, string str2)
	{
		makeLower(&str1);
		makeLower(&str2);
		return str1 == str2;
	}

	bool compareIns(wstring str1, wstring str2)
	{
		makeLower(&str1);
		makeLower(&str2);
		return str1 == str2;
	}
}

bool NormalizeRegistryPath(wstring* KeyPath)
{
	bool result = true;
	if (str_util::startsWith(KeyPath, L"HKEY_LOCAL_MACHINE"))
		KeyPath->replace(0, RTL_NUMBER_OF(L"HKEY_LOCAL_MACHINE"), L"\\Registry\\Machine\\");
	else if (str_util::startsWith(KeyPath, L"HKEY_USERS"))
		KeyPath->replace(0, RTL_NUMBER_OF(L"HKEY_USERS"), L"\\Registry\\Users\\");
	else
	{
		kprintf(TRACE_INFO, "Key path format not supported");
		result = false;
	}

	return result;
}

