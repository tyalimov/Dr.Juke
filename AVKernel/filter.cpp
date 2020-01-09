
#include "filter.h"
#include "util.h"
#include <EASTL/string.h>

//------------------------------------------------------------>
// Prototypes

NTSTATUS RegFilterCallback(
	_In_     PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2);

NTSTATUS RegPostSetValueKey(
	_In_ PREGFILTER_CALLBACK_CTX CbContext,
	_Inout_	PVOID Argument2,
	_In_ BOOLEAN bDeleted);

NTSTATUS RegPreCreateKeyEx(
	_In_ PREGFILTER_CALLBACK_CTX CallbackCtx,
	_Inout_ PVOID Argument2);


//------------------------------------------------------------>
// Registry filter

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject, PREGFILTER_CALLBACK_CTX CbContext)
{
	NTSTATUS Status;
	ULONG MajorVersion = 0, MinorVersion = 0;

    CmGetCallbackVersion(&MajorVersion, &MinorVersion);
    kprintf(TRACE_REGFILTER, "Callback version %u.%u", MajorVersion, MinorVersion);

	if (CbContext->OnRegFilterInit)
	{
		Status = CbContext->OnRegFilterInit(CbContext);
		if (!NT_SUCCESS(Status))
			goto exit;
	}

	Status = CmRegisterCallbackEx(
		RegFilterCallback,
		&CbContext->Altitude,
		DriverObject,
		(PVOID)CbContext,
		&CbContext->Cookie,
		NULL);

exit:
	kprint_st(TRACE_REGFILTER, Status);
	return Status;
}

void RegFilterExit(PREGFILTER_CALLBACK_CTX CbContext)
{
	NTSTATUS Status = CmUnRegisterCallback(CbContext->Cookie);
	
	if (CbContext->OnRegFilterExit)
		CbContext->OnRegFilterExit(CbContext);
	
	kprint_st(TRACE_REGFILTER, Status);
}

NTSTATUS
RegFilterCallback(
	_In_  PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	REG_NOTIFY_CLASS NotifyClass;
	PREGFILTER_CALLBACK_CTX CbContext;

	CbContext = (PREGFILTER_CALLBACK_CTX)CallbackContext;
	NotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;

	if (Argument2 == NULL)
	{
		kprintf(TRACE_REGFILTER, "Argument 2 unexpectedly 0. Abort with STATUS_SUCCESS");
		return STATUS_SUCCESS;
	}

	switch (NotifyClass)
	{
	case RegNtPreCreateKeyEx: 
	case RegNtPreOpenKeyEx: 
		Status = RegPreCreateKeyEx(CbContext, Argument2);
		break;
	case RegNtPostSetValueKey: 
		Status = RegPostSetValueKey(CbContext, Argument2, FALSE);
		break;
	case RegNtPostDeleteValueKey:
		Status = RegPostSetValueKey(CbContext, Argument2, TRUE);
		break;
	default: 
		break;
	}

	return Status;
}

NTSTATUS 
RegPreCreateKeyEx(
    _In_ PREGFILTER_CALLBACK_CTX CbContext,
    _Inout_	PVOID Argument2)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PREG_CREATE_KEY_INFORMATION_V1 PreCreateInfo;
	  
	PreCreateInfo = (PREG_CREATE_KEY_INFORMATION_V1) Argument2;
	if ((ULONG_PTR)PreCreateInfo->Version == 1)
	{
		if (CbContext->OnRegNtPreCreateKeyEx)
			Status = CbContext->OnRegNtPreCreateKeyEx(PreCreateInfo, CbContext);
	}
	else
		kprintf(TRACE_REGFILTER, "PreCreateInfo type is not PREG_CREATE_KEY_INFORMATION_V1");

    return Status;
}

NTSTATUS RegPostSetValueKey(
	_In_ PREGFILTER_CALLBACK_CTX CbContext,
	_Inout_	PVOID Argument2,
	_In_ BOOLEAN bDeleted)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PREG_POST_OPERATION_INFORMATION PostInfo;
	PostInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;

	if (CbContext->OnRegNtPostSetValueKey)
		Status = CbContext->OnRegNtPostSetValueKey(PostInfo, CbContext, bDeleted);

	return Status;
}
