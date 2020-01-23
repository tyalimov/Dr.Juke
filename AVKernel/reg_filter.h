#pragma once
#include "common.h"

#define KRF_PROTECTED_KEYS L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ProtectedKeys"
#define KRF_ALLOWED_PROCESSES L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\AllowedProcesses" 

#define REGFILTER_ALTITUDE L"380010"

struct REGFILTER_CALLBACK_CTX;
using PREGFILTER_CALLBACK_CTX = REGFILTER_CALLBACK_CTX*;

using RegNotifyKeyValueChangeRoutine = void(*)(
	PREG_POST_OPERATION_INFORMATION,
	PREGFILTER_CALLBACK_CTX,
	BOOLEAN);

struct REGFILTER_CALLBACK_CTX
{
	LARGE_INTEGER Cookie = { 0 };
	BOOLEAN IsInitialized = FALSE;

	RegNotifyKeyValueChangeRoutine onKeyChange = nullptr;
};


void RegFilterExit(PREGFILTER_CALLBACK_CTX CbContext);

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject, PREGFILTER_CALLBACK_CTX CbContext);

bool RegFilterAddProtectedKey(PWCH Name,
	ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type);

void RegFilterRemoveProtectedKey(PWCH Name, ULONG NameLen);

bool RegFilterAddAllowedProcess(PWCH Name, ULONG NameLen);

bool RegFilterRemoveAllowedProcess(PWCH Name, ULONG NameLen);
