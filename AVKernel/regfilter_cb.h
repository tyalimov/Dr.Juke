#pragma once

#include "filter.h"

#define KEY_PROCFILTER L"\\Registry\\Machine\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\ProcFilter" 
#define KEY_TOTALCMD L"\\Registry\\Machine\\SOFTWARE\\Ghisler\\Total Commander"

NTSTATUS OnRegFilterInit(
	PREGFILTER_CALLBACK_CTX CbContext);

NTSTATUS OnRegNtPostSetValueKey(
	PREG_POST_OPERATION_INFORMATION,
	PREGFILTER_CALLBACK_CTX);

NTSTATUS OnRegNtPreCreateKeyEx(
	PREG_CREATE_KEY_INFORMATION_V1,
	PREGFILTER_CALLBACK_CTX);
