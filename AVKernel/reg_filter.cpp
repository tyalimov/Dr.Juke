#include "reg_filter.h"
#include "preferences.h"
#include "util.h"
#include "access_monitor.h"

//------------------------------------------------------------>
// Prototypes

static NTSTATUS 
RegFilterCallback(
	_In_     PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2);

static NTSTATUS 
RegPostSetValueKey(
	_Inout_	PVOID Argument2,
	_In_ BOOLEAN bDeleted);

static NTSTATUS 
RegPreCreateKeyEx(_Inout_ PVOID Argument2);


//------------------------------------------------------------>
// Registry filter

PRegistryAccessMonitor gRegMon = nullptr;
LARGE_INTEGER gCookie = { 0 };

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG MajorVersion = 0, MinorVersion = 0;
	UNICODE_STRING Altitude = RTL_CONSTANT_STRING(REGFILTER_ALTITUDE);

    CmGetCallbackVersion(&MajorVersion, &MinorVersion);
    kprintf(TRACE_REGFILTER_INFO, "Callback version %u.%u", MajorVersion, MinorVersion);

	gRegMon = new RegistryAccessMonitor(KRF_BASE_KEY);
	if (gRegMon)
	{
		Status = CmRegisterCallbackEx(
			RegFilterCallback,
			&Altitude,
			DriverObject,
			NULL,
			&gCookie,
			NULL);

		if (!NT_SUCCESS(Status))
		{
			delete gRegMon;
			gRegMon = nullptr;
		}
	}

	kprint_st(TRACE_REGFILTER_INFO, Status);
	return Status;
}

void RegFilterExit()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	
	if (gRegMon)
	{
		Status = CmUnRegisterCallback(gCookie);

		delete gRegMon;
		gRegMon = nullptr;
	}

	kprint_st(TRACE_REGFILTER_INFO, Status);
}

static NTSTATUS
RegFilterCallback(
	_In_  PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	REG_NOTIFY_CLASS NotifyClass;

	UNREFERENCED_PARAMETER(CallbackContext);
	NotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;

	// Ignore windows system process
	if (PsGetCurrentProcessId() == (PID)4)
		return STATUS_SUCCESS;

	if (Argument2 == NULL)
	{
		kprintf(TRACE_REGFILTER_INFO, "Argument 2 unexpectedly 0. Abort with STATUS_SUCCESS");
		return STATUS_SUCCESS;
	}

	switch (NotifyClass)
	{
	case RegNtPreCreateKeyEx: 
	case RegNtPreOpenKeyEx: 
		Status = RegPreCreateKeyEx(Argument2);
		break;
	case RegNtPostSetValueKey: 
		Status = RegPostSetValueKey(Argument2, FALSE);
		break;
	case RegNtPostDeleteValueKey:
		Status = RegPostSetValueKey(Argument2, TRUE);
		break;
	default: 
		break;
	}

	return Status;
}

static NTSTATUS 
RegPreCreateKeyEx(_Inout_ PVOID Argument2)
{
	NTSTATUS Status;
    PREG_CREATE_KEY_INFORMATION_V1 PreCreateInfo;
	  
	PreCreateInfo = (PREG_CREATE_KEY_INFORMATION_V1) Argument2;
	if ((ULONG_PTR)PreCreateInfo->Version != 1)
	{
		kprintf(TRACE_REGFILTER_INFO, "PreCreateInfo type is not PREG_CREATE_KEY_INFORMATION_V1");
		return STATUS_SUCCESS;
	}

	PUNICODE_STRING usKeyPath = PreCreateInfo->CompleteName;
	
	wstring KeyPath(usKeyPath->Buffer, usKeyPath->Length / sizeof(WCHAR));
	if (KeyPath[0] != L'\\')
	{
		// Convert relative path to absolute
		PCUNICODE_STRING usObjectPath;
		Status = CmCallbackGetKeyObjectID(&gCookie,
			PreCreateInfo->RootObject, NULL, &usObjectPath);

		if (!NT_SUCCESS(Status))
		{
			kprintf(TRACE_REGFILTER_INFO, "CmCallbackGetKeyObjectID failed with status=0x%08X", Status);
			return STATUS_SUCCESS;
		}
		else
		{
			wstring ObjectPath(usObjectPath->Buffer, usObjectPath->Length / sizeof(WCHAR));
			ObjectPath.append(L"\\");
			KeyPath.insert(0, move(ObjectPath));
		}
	}

	PID pid = PsGetCurrentProcessId();
	bool allowed = gRegMon->isAccessAllowed(pid, 
		KeyPath, PreCreateInfo->DesiredAccess);

	return allowed ? STATUS_SUCCESS : STATUS_ACCESS_DENIED;
}

static NTSTATUS 
RegPostSetValueKey(
	_Inout_	PVOID Argument2,
	_In_ BOOLEAN bDeleted)
{
	NTSTATUS Status;
	PCUNICODE_STRING KeyPath;
	PREG_SET_VALUE_KEY_INFORMATION PreInfo;
	PREG_POST_OPERATION_INFORMATION PostInfo;
	PostInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
    PreInfo = (PREG_SET_VALUE_KEY_INFORMATION)PostInfo->PreInformation;

	// Ignore failed operations
	if (!NT_SUCCESS(PostInfo->Status))
		return STATUS_SUCCESS;

	// Get Absolute key name path
	Status = CmCallbackGetKeyObjectID(&gCookie, 
		PostInfo->Object, NULL, &KeyPath);

	if (!NT_SUCCESS(Status))
	{
		kprintf(TRACE_REGFILTER_INFO, "CmCallbackGetKeyObjectID failed with status=0x%08X", Status);
		return STATUS_SUCCESS;
	}
	
	UNICODE_STRING JukeKey = RTL_CONSTANT_STRING(DR_JUKE_BASE_KEY);
	BOOLEAN res = RtlPrefixUnicodeString(&JukeKey, KeyPath, FALSE);
	//kprintf(TRACE_INFO, "RtlPrefixUnicodeString res=%d %wZ, %wZ", res, JukeKey, KeyPath);

	// Not interested in modified keys 
	// other than Dr.Juke ones
	if (!res)
		return STATUS_SUCCESS;

	wstring key_path(KeyPath->Buffer, KeyPath->Length / sizeof(WCHAR));
	gRegMon->onRegKeyChange(key_path, PreInfo, bDeleted);

	return STATUS_SUCCESS;
}

