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

bool IsWin8OrGreater()
{
    RTL_OSVERSIONINFOEXW VersionInfo = {0};
    ULONGLONG ConditionMask = 0;
    NTSTATUS Status;
    bool Result;

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
        Result = false;
    else if (Status == STATUS_REVISION_MISMATCH) 
        Result = true;
    else 
    {
        kprintf(TRACE_ERROR, "RtlVerifyVersionInfo returned unexpected status 0x%08X", Status);
        Result = false;  
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
	bool startsWith(const wstring& str, const wchar_t* substr) {
		auto n = wcslen(substr);
		return (str.length() >= n) &&
			(wcsncmp(str.c_str(), substr, n) == 0);
	}

	bool startsWith(const string& str, const char* substr) {
		auto n = strlen(substr);
		return (str.length() >= n) &&
			(strncmp(str.c_str(), substr, n) == 0);
	}

	bool endsWith(const wstring& str, const wchar_t* substr)
	{
		auto n = wcslen(substr);
		auto len = str.length(); 
		return (len >= n) &&
			(wcsncmp(str.c_str() + len - n, substr, n) == 0);
	}

	bool endsWith(const string& str, const char* substr)
	{
		auto n = strlen(substr);
		auto len = str.length(); 
		return (len >= n) &&
			(strncmp(str.c_str() + len - n, substr, n) == 0);
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


	ANSI_STRING ToAnsiString(string& str)
	{
		ANSI_STRING ansi_str;
		ansi_str.Buffer = (PCHAR)str.c_str();
		ansi_str.Length = (USHORT)str.length();
		ansi_str.MaximumLength = ansi_str.Length + 1;
		return ansi_str;
	}

	string FromAnsiString(const ANSI_STRING& ansi_str)
	{
		return string(ansi_str.Buffer, ansi_str.Length);
	}

	string FromAnsiString(const ANSI_STRING* ansi_str)
	{
		return string(ansi_str->Buffer, ansi_str->Length);
	}

	//
	// Warning!
	// Notice, that when wstring object is freed, the UNICODE_STRING buffer is freed too
	// Use created UNICODE_STRING object only when wstring object is present!
	//
	// Don't use it like this: UNICODE_STRING us = mstl::WStringToUnicodeString(L"aaaa");
	//
	// Usage:
	//		string ws = L"12345";
	//		UNICODE_STRING us = WStringToUnicodeString(ws);
	//		...
	//
	UNICODE_STRING ToUnicodeString(wstring& wstr)
	{
		UNICODE_STRING uni_str;
		uni_str.Buffer = (PWCHAR)wstr.c_str();
		uni_str.Length = (USHORT)wstr.length() * sizeof(wchar_t);
		uni_str.MaximumLength = uni_str.Length + 1 * sizeof(wchar_t);
		return uni_str;
	}

	// Usage:
	//
	//  C_UNICODE_STRING(us, L"12345");
	//	wstring ws = UnicodeStringToWString(us);
	//	...
	//
	wstring FromUnicodeString(const UNICODE_STRING& uni_str)
	{
		return wstring(uni_str.Buffer, uni_str.Length / sizeof(wchar_t));
	}

	wstring FromUnicodeString(const UNICODE_STRING* uni_str)
	{
		return wstring(uni_str->Buffer, uni_str->Length / sizeof(wchar_t));
	}

	// Usage:
	//
	//	string s = "12345";
	//	wstring ws = StringToWString(ws);
	//	...
	//
	wstring ToWString(string& str)
	{
		UNICODE_STRING uni_str = { 0 };
		ANSI_STRING ansi_str = ToAnsiString(str);
		RtlAnsiStringToUnicodeString(&uni_str, &ansi_str, TRUE);

		wstring wstr = FromUnicodeString(uni_str);
		RtlFreeUnicodeString(&uni_str);
		return wstr;
	}

	// Usage:
	//
	//	wstring ws = L"12345";
	//	string s = WStringToString(ws);
	//	...
	//
	string ToString(wstring& wstr)
	{
		ANSI_STRING ansi_str = { 0 };
		UNICODE_STRING uni_str = ToUnicodeString(wstr);
		RtlUnicodeStringToAnsiString(&ansi_str, &uni_str, TRUE);

		string str = FromAnsiString(ansi_str);
		RtlFreeAnsiString(&ansi_str);
		return str;
	}
}

