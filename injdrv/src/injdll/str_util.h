#pragma once

//
// Include NTDLL-related headers.
//
#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>

#include "string.h"

using _snwprintf_fn_t = int(__cdecl*)(
	wchar_t* buffer,
	size_t count,
	const wchar_t* format,
	...
	);

using _tolower_t = decltype(tolower)*;
using _towlower_t = decltype(towlower)*;

namespace ownstl
{
	ANSI_STRING ToAnsiString(const string& str);

	string FromAnsiString(const ANSI_STRING& ansi_str);

	string FromAnsiString(const ANSI_STRING* ansi_str);
	
	UNICODE_STRING ToUnicodeString(const wstring& wstr);

	wstring FromUnicodeString(const UNICODE_STRING& uni_str);

	wstring FromUnicodeString(const UNICODE_STRING* uni_str);

	wstring ToWString(const string& str);

	string ToString(const wstring& wstr);

	///////////// ANSI -- char

	bool operator==(const ANSI_STRING& left, const char* right);

	bool operator!=(const ANSI_STRING& left, const char* right);

	bool operator==(const ANSI_STRING& left, const string& right);

	bool operator!=(const ANSI_STRING& left, const string& right);

	///////////// UNICODE -- wchar_t

	bool operator==(const UNICODE_STRING& left, const wchar_t* right);

	bool operator!=(const UNICODE_STRING& left, const wchar_t* right);

	bool operator==(const UNICODE_STRING& left, const wstring& right);

	bool operator!=(const UNICODE_STRING& left, const wstring& right);

	///////////// ANSI -- ANSI

	bool operator==(const ANSI_STRING& left, const ANSI_STRING& right);

	bool operator!=(const ANSI_STRING& left, const ANSI_STRING& right);

	///////////// UNICODE -- UNICODE

	bool operator==(const UNICODE_STRING& left, const UNICODE_STRING& right);

	bool operator!=(const UNICODE_STRING& left, const PUNICODE_STRING& right);

	///////////// lower upper

	wstring ToLowerCase(wstring ws);

	string ToLowerCase(string s);
}
