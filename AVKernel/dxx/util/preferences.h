#pragma once

#include "common.h"
#include <EASTL/functional.h>

NTSTATUS PreferencesReadFull(LPCWSTR AbsRegPath,
	eastl::function<void(PWCH Name, ULONG NameLength, PVOID Data, ULONG DataLength, ULONG Type)> onRecord);

NTSTATUS PreferencesReadBasic(LPCWSTR AbsRegPath,
	eastl::function<void(PWCH Name, ULONG NameLength)> onRecord);

NTSTATUS PreferencesQueryKeyValue(const wchar_t* szAbsRegPath,
	const wchar_t* szValueName, eastl::function<NTSTATUS(PKEY_VALUE_FULL_INFORMATION)> onRecord);
