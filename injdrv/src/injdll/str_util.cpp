#include "str_util.h"

namespace ownstl
{
	//
	// Warning!
	// Notice, that when string object is freed, the ANSI_STRING buffer is freed too
	// Use created ANSI_STRING object only when string object is present!
	//
	// Don't use it like this: ANSI_STRING us = mstl::StringToAnsiString("aaaa");
	//
	// Usage:
	//		string s = "12345";
	//		ANSI_STRING as = StringToAnsiString(s);
	//		...
	//
	ANSI_STRING ToAnsiString(const string& str)
	{
		ANSI_STRING ansi_str;
		ansi_str.Buffer = (PCHAR)str.c_str();
		ansi_str.Length = (USHORT)str.length();
		ansi_str.MaximumLength = ansi_str.Length + 1;
		return ansi_str;
	}

	// Usage:
	//		C_ANSI_STRING(as, "12345");
	//		string s = AnsiStringToString(as);
	//		...
	//
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
	UNICODE_STRING ToUnicodeString(const wstring& wstr)
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
	wstring ToWString(const string& str)
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
	string ToString(const wstring& wstr)
	{
		ANSI_STRING ansi_str = { 0 };
		UNICODE_STRING uni_str = ToUnicodeString(wstr);
		RtlUnicodeStringToAnsiString(&ansi_str, &uni_str, TRUE);

		string str = FromAnsiString(ansi_str);
		RtlFreeAnsiString(&ansi_str);
		return str;
	}


	///////////// ANSI -- char

	bool operator==(const ANSI_STRING& left, const char* right)
	{
		size_t length = string::strlen(right);

		if (length != left.Length)
			return false;

		return string::strncmp(left.Buffer, right, length) == 0;
	}

	bool operator!=(const ANSI_STRING& left, const char* right)
	{
		return !(left == right);
	}

	bool operator==(const ANSI_STRING& left, const string& right)
	{
		return left == right.c_str();
	}

	bool operator!=(const ANSI_STRING& left, const string& right)
	{
		return !(left == right);
	}

	///////////// UNICODE -- wchar_t

	bool operator==(const UNICODE_STRING& left, const wchar_t* right)
	{
		size_t length = wstring::strlen(right);

		if (length * sizeof(wchar_t) != left.Length)
			return false;

		return wstring::strncmp(left.Buffer, right, length) == 0;
	}

	bool operator!=(const UNICODE_STRING& left, const wchar_t* right)
	{
		return !(left == right);
	}

	bool operator==(const UNICODE_STRING& left, const wstring& right)
	{
		return left == right.c_str();
	}

	bool operator!=(const UNICODE_STRING& left, const wstring& right)
	{
		return !(left == right);
	}

	///////////// ANSI -- ANSI

	bool operator==(const ANSI_STRING& left, const ANSI_STRING& right)
	{
		return RtlEqualString((PSTRING)& left, (PSTRING)& right, FALSE);
	}

	bool operator!=(const ANSI_STRING& left, const ANSI_STRING& right)
	{
		return !(left == right);
	}

	///////////// UNICODE -- UNICODE

	bool operator==(const UNICODE_STRING& left, const UNICODE_STRING& right)
	{
		return RtlEqualUnicodeString((PUNICODE_STRING)& left, (PUNICODE_STRING)& right, FALSE);
	}

	bool operator!=(const UNICODE_STRING& left, const PUNICODE_STRING& right)
	{
		return !(left == right);
	}
}
