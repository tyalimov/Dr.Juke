#pragma once

#include "common.h"
#include <EASTL/functional.h>

#define KEY_PROCFILTER L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\ProcFilter" 
#define KEY_TOTALCMD L"\\Registry\\Machine\\SOFTWARE\\Ghisler\\Total Commander"

NTSTATUS PreferencesReadFull(LPCWSTR AbsRegPath,
	eastl::function<void(PWCH Name, ULONG NameLength, PVOID Data, ULONG DataLength, ULONG Type)> onRecord);

NTSTATUS PreferencesReadBasic(LPCWSTR AbsRegPath,
	eastl::function<void(PWCH Name, ULONG NameLength)> onRecord);

NTSTATUS PreferencesReset(LPCWSTR AbsRegPath);
