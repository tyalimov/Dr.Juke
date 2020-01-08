#pragma once

#include "common.h"
#include <EASTL/string.h>

#define REGFILTER_ALTITUDE L"380010"

struct REGFILTER_CALLBACK_CTX;
using PREGFILTER_CALLBACK_CTX = REGFILTER_CALLBACK_CTX*;

using RegNtPostSetValueKeyRoutine = NTSTATUS(*)(
	PREG_POST_OPERATION_INFORMATION,
	PREGFILTER_CALLBACK_CTX);

using RegNtPreCreateKeyExRoutine = NTSTATUS(*)(
	PREG_CREATE_KEY_INFORMATION_V1,
	PREGFILTER_CALLBACK_CTX);

using RegFilterInitRoutine = NTSTATUS(*)(
	PREGFILTER_CALLBACK_CTX);

using RegFilterExitRoutine = void(*)(
	PREGFILTER_CALLBACK_CTX);

struct REGFILTER_CALLBACK_CTX
{
	LARGE_INTEGER Cookie = { 0 };
	UNICODE_STRING Altitude = { 0 };
	RegNtPostSetValueKeyRoutine OnRegNtPostSetValueKey = nullptr;
	RegNtPreCreateKeyExRoutine OnRegNtPreCreateKeyEx = nullptr;
	RegFilterInitRoutine OnRegFilterInit = nullptr;
	RegFilterExitRoutine OnRegFilterExit = nullptr;
};


void RegFilterExit(PREGFILTER_CALLBACK_CTX CbContext);

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject, PREGFILTER_CALLBACK_CTX CbContext);
