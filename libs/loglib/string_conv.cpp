#include "string_conv.h"
#include <codecvt>

#pragma warning ( disable:4996 )

#define ERROR_WIDE_STRING  L""
#define ERROR_BYTES_STRING ""

static std::wstring_convert<std::codecvt_utf8<wchar_t> > utf8conv(ERROR_BYTES_STRING, ERROR_WIDE_STRING);


std::wstring ToWstring(const std::string& text)
{
	return utf8conv.from_bytes(text);
}

std::wstring ToWstring(const char* text)
{
	return utf8conv.from_bytes(text);
}