#pragma once

#include "filter.h"

#define KRF_PROTECTED_KEYS L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\ProtectedKeys"
#define KRF_ALLOWED_PROCESSES L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter\\AllowedProcesses" 

NTSTATUS OnRegFilterInit(
	PREGFILTER_CALLBACK_CTX CbContext);

NTSTATUS OnRegNtPostSetValueKey(
	PREG_POST_OPERATION_INFORMATION,
	PREGFILTER_CALLBACK_CTX,
	BOOLEAN);

NTSTATUS OnRegNtPreCreateKeyEx(
	PREG_CREATE_KEY_INFORMATION_V1,
	PREGFILTER_CALLBACK_CTX);
