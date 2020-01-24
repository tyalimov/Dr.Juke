#pragma once
#include "common.h"

#define KRF_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter"

#define KRF_PROTECTED_KEYS L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ProtectedKeys"
#define KRF_ALLOWED_PROCESSES L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\AllowedProcesses" 

#define REGFILTER_ALTITUDE L"380010"

void RegFilterExit();

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject);

bool RegFilterAddProtectedKey(PWCH Name,
	ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type);

void RegFilterRemoveProtectedKey(PWCH Name, ULONG NameLen);

bool RegFilterAddAllowedProcess(PWCH Name, ULONG NameLen);

bool RegFilterRemoveAllowedProcess(PWCH Name, ULONG NameLen);
