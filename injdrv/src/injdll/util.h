#pragma once

#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>

VOID ProtectDll(HANDLE ModuleHandle);

NTSTATUS QueryKeyValue(const wchar_t* szAbsRegPath,
	const wchar_t* szValueName, NTSTATUS(*onRecord)(PKEY_VALUE_FULL_INFORMATION));

PUNICODE_STRING CurrentProcessImagePath();
