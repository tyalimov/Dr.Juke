
#include "filter.h"
#include "util.h"
#include <EASTL/string.h>

//------------------------------------------------------------>
// Prototypes

NTSTATUS RegCallback(
	_In_     PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2);

NTSTATUS RegPreCreateKey(
	_In_ PREGFILTER_CALLBACK_CTX CbContext,
	_Inout_	PVOID Argument2);

NTSTATUS RegPreCreateKeyEx(
	_In_ PREGFILTER_CALLBACK_CTX CallbackCtx,
	_Inout_ PVOID Argument2);

LPCWSTR GetNotifyClassString(REG_NOTIFY_CLASS NotifyClass);

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
		RegCallback,
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
RegCallback(
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

	//UNICODE_STRING us;
	//RtlInitUnicodeString(&us, L"");
	//RtlCompareUnicodeString(&us, )
	//if ()


	switch (NotifyClass)
	{
	case RegNtPreCreateKeyEx: 
	case RegNtPreOpenKeyEx: 
		Status = RegPreCreateKeyEx(CbContext, Argument2);
		break;
	case RegNtPreCreateKey:
		Status = RegPreCreateKey(CbContext, Argument2);
		break;

	case RegNtPostSetValueKey: 
	//PREG_POST_OPERATION_INFORMATION PostSetKeyValueInfo;
		break;
	default: 
		break;
	}

	return Status;
}

NTSTATUS 
RegPreCreateKey(
    _In_ PREGFILTER_CALLBACK_CTX CbContext,
    _Inout_	PVOID Argument2
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PREG_CREATE_KEY_INFORMATION PreCreateInfo;
	PreCreateInfo = (PREG_CREATE_KEY_INFORMATION) Argument2;
	
	UNREFERENCED_PARAMETER(CbContext);

	kprintf(TRACE_INFO, "Access to %s", PreCreateInfo->CompleteName);

    return Status;
}

NTSTATUS 
RegPreCreateKeyEx(
    _In_ PREGFILTER_CALLBACK_CTX CbContext,
    _Inout_	PVOID Argument2
    )
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

LPCWSTR GetNotifyClassString(REG_NOTIFY_CLASS NotifyClass)
{
	switch (NotifyClass) 
	{
	case RegNtPreDeleteKey:                 return L"RegNtPreDeleteKey";
	case RegNtPreSetValueKey:               return L"RegNtPreSetValueKey";
	case RegNtPreDeleteValueKey:            return L"RegNtPreDeleteValueKey";
	case RegNtPreSetInformationKey:         return L"RegNtPreSetInformationKey";
	case RegNtPreRenameKey:                 return L"RegNtPreRenameKey";
	case RegNtPreEnumerateKey:              return L"RegNtPreEnumerateKey";
	case RegNtPreEnumerateValueKey:         return L"RegNtPreEnumerateValueKey";
	case RegNtPreQueryKey:                  return L"RegNtPreQueryKey";
	case RegNtPreQueryValueKey:             return L"RegNtPreQueryValueKey";
	case RegNtPreQueryMultipleValueKey:     return L"RegNtPreQueryMultipleValueKey";
	case RegNtPreKeyHandleClose:            return L"RegNtPreKeyHandleClose";
	case RegNtPreCreateKeyEx:               return L"RegNtPreCreateKeyEx";
	case RegNtPreOpenKeyEx:                 return L"RegNtPreOpenKeyEx";
	case RegNtPreFlushKey:                  return L"RegNtPreFlushKey";
	case RegNtPreLoadKey:                   return L"RegNtPreLoadKey";
	case RegNtPreUnLoadKey:                 return L"RegNtPreUnLoadKey";
	case RegNtPreQueryKeySecurity:          return L"RegNtPreQueryKeySecurity";
	case RegNtPreSetKeySecurity:            return L"RegNtPreSetKeySecurity";
	case RegNtPreRestoreKey:                return L"RegNtPreRestoreKey";
	case RegNtPreSaveKey:                   return L"RegNtPreSaveKey";
	case RegNtPreReplaceKey:                return L"RegNtPreReplaceKey";

	case RegNtPostDeleteKey:                return L"RegNtPostDeleteKey";
	case RegNtPostSetValueKey:              return L"RegNtPostSetValueKey";
	case RegNtPostDeleteValueKey:           return L"RegNtPostDeleteValueKey";
	case RegNtPostSetInformationKey:        return L"RegNtPostSetInformationKey";
	case RegNtPostRenameKey:                return L"RegNtPostRenameKey";
	case RegNtPostEnumerateKey:             return L"RegNtPostEnumerateKey";
	case RegNtPostEnumerateValueKey:        return L"RegNtPostEnumerateValueKey";
	case RegNtPostQueryKey:                 return L"RegNtPostQueryKey";
	case RegNtPostQueryValueKey:            return L"RegNtPostQueryValueKey";
	case RegNtPostQueryMultipleValueKey:    return L"RegNtPostQueryMultipleValueKey";
	case RegNtPostKeyHandleClose:           return L"RegNtPostKeyHandleClose";
	case RegNtPostCreateKeyEx:              return L"RegNtPostCreateKeyEx";
	case RegNtPostOpenKeyEx:                return L"RegNtPostOpenKeyEx";
	case RegNtPostFlushKey:                 return L"RegNtPostFlushKey";
	case RegNtPostLoadKey:                  return L"RegNtPostLoadKey";
	case RegNtPostUnLoadKey:                return L"RegNtPostUnLoadKey";
	case RegNtPostQueryKeySecurity:         return L"RegNtPostQueryKeySecurity";
	case RegNtPostSetKeySecurity:           return L"RegNtPostSetKeySecurity";
	case RegNtPostRestoreKey:               return L"RegNtPostRestoreKey";
	case RegNtPostSaveKey:                  return L"RegNtPostSaveKey";
	case RegNtPostReplaceKey:               return L"RegNtPostReplaceKey";

	case RegNtCallbackObjectContextCleanup: return L"RegNtCallbackObjectContextCleanup";

	default:
		return L"Unsupported REG_NOTIFY_CLASS";
	}
}

