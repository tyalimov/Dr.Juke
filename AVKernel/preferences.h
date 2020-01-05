#pragma once

#include "common.h"
#include <EASTL/functional.h>

#define KEY_APPS L"\\Registry\\Machine\\SOFTWARE\\RegisteredApplications" 
#define KEY_TOTALCMD L"\\Registry\\Machine\\SOFTWARE\\Ghisler\\Total Commander"

NTSTATUS PreferencesReadFull(LPCWSTR AbsRegPath,
	eastl::function<void(PWCH Name, ULONG NameLength, PVOID Data, ULONG DataLength, ULONG Type)> onRecord);
