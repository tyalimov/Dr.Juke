#include "fs_filter.h"
#include <fltKernel.h>
#include "common.h"
#include <EASTL/string.h>

using namespace eastl;


FLT_PREOP_CALLBACK_STATUS FltCreatePreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

const FLT_OPERATION_REGISTRATION Callbacks[] = {
	{ IRP_MJ_CREATE, 0, FltCreatePreOperation,  /*FltCreatePostOperation*/ NULL },
	{ IRP_MJ_OPERATION_END }
};

NTSTATUS
FsFilterInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
FsFilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
FsFilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

VOID
FsFilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
FsFilterExit (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

PFileSystemAccessMonitor gFsMon = nullptr;
GuardedMutex gFsMonLock;

PFileSystemAccessMonitor FsFilterGetInstancePtr() 
{
	LockGuard<GuardedMutex> guard(&gFsMonLock);
	return gFsMon;
}

bool FsFilterNewInstance()
{
	LockGuard<GuardedMutex> guard(&gFsMonLock);

	gFsMon = new FileSystemAccessMonitor(KFF_BASE_KEY);
	gFsMon->regReadConfiguration();
	return gFsMon != nullptr;
}

void FsFilterDeleteInstance()
{
	gFsMonLock.acquire();
	delete gFsMon;
	gFsMon = nullptr;
	gFsMonLock.release();
}

PFLT_FILTER gFilterHandle = nullptr;

CONST FLT_REGISTRATION FilterRegistration = {

	sizeof(FLT_REGISTRATION),           //  Size
	FLT_REGISTRATION_VERSION,           //  Version
#if OS_WIN8_OR_GREATER 
    FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,   //  Flags
#else
    0,                                      //  Flags
#endif // MINISPY_WIN8

	NULL,                               //  Context
	Callbacks,                          //  Operation callbacks
	FsFilterExit,                       //  MiniFilterUnload
	FsFilterInstanceSetup,              //  InstanceSetup
	FsFilterInstanceQueryTeardown,      //  InstanceQueryTeardown
	FsFilterInstanceTeardownStart,      //  InstanceTeardownStart
	FsFilterInstanceTeardownComplete,   //  InstanceTeardownComplete

	NULL,                               //  GenerateFileName
	NULL,                               //  GenerateDestinationFileName
	NULL                                //  NormalizeNameComponent
};



NTSTATUS FsFilterInstanceSetup(
	PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_SETUP_FLAGS Flags, 
	DEVICE_TYPE VolumeDeviceType, FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
	UNREFERENCED_PARAMETER(FltObjects);

	if (VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM
		|| VolumeDeviceType == FILE_DEVICE_NAMED_PIPE)
	{
		kprintf(TRACE_FSFILTER, "Attached to new volume <flags:0x%08x, device:0x%08x, fs:0x%08x>",
			(ULONG)Flags, (ULONG)VolumeDeviceType, (ULONG)VolumeFilesystemType);

		return STATUS_SUCCESS;
	}

	return STATUS_FLT_DO_NOT_ATTACH;
}

FLT_PREOP_CALLBACK_STATUS FltCreatePreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext)
{
	NTSTATUS Status;
	ACCESS_MASK DesiredAccess;
	PFLT_FILE_NAME_INFORMATION fltFileInfo;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	// Ignore windows system process
	PID CurrentProcessId = PsGetCurrentProcessId();
	if (CurrentProcessId == (PID)4)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	DesiredAccess = Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;
	Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED, &fltFileInfo);

	if (!NT_SUCCESS(Status))
	{
		if (Status != STATUS_OBJECT_PATH_NOT_FOUND)
		{
			kprintf(TRACE_ERROR, "FltGetFileNameInformation() "
				"failed <Status=0x%08X>", Status);
		}

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	//_FLT_RELATED_OBJECTS -> Volume -> FltQueryVolumeInformation -> FileFsDeviceInformation -> DEVICE_TYPE

	bool allowed = true;
	PUNICODE_STRING usName = &fltFileInfo->Name;

	if (gFsMon != nullptr)
	{
		wstring name(usName->Buffer, usName->Length / sizeof(WCHAR));
		allowed = gFsMon->isAccessAllowed(CurrentProcessId, name, DesiredAccess);
	}

	FltReleaseFileNameInformation(fltFileInfo);

	if (!allowed)
	{
		Data->IoStatus.Status = STATUS_ACCESS_DENIED;
		return FLT_PREOP_COMPLETE;
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS FsFilterInit(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS Status;

	bool ok = FsFilterNewInstance();
	if (!ok)
		Status = STATUS_INSUFFICIENT_RESOURCES;
	else
	{
		Status = FltRegisterFilter(DriverObject,
			&FilterRegistration, &gFilterHandle);

		if (NT_SUCCESS(Status))
		{
			Status = FltStartFiltering(gFilterHandle);
			if (!NT_SUCCESS(Status))
			{
				FltUnregisterFilter(gFilterHandle);
				gFilterHandle = nullptr;

				FsFilterDeleteInstance();
			}
		}
	}

	kprint_st(TRACE_FSFILTER, Status);
	return Status;
}

NTSTATUS FsFilterExit(FLT_FILTER_UNLOAD_FLAGS Flags)
{
	UNREFERENCED_PARAMETER(Flags);
	NTSTATUS Status = STATUS_SUCCESS;

	if (FlagOn(FLTFL_FILTER_UNLOAD_MANDATORY, Flags))
	{
		if (gFilterHandle != nullptr)
		{
			FltUnregisterFilter(gFilterHandle);
			gFilterHandle = nullptr;

			FsFilterDeleteInstance();
		}
	}
	else
		Status = STATUS_FLT_DO_NOT_DETACH;

	kprint_st(TRACE_FSFILTER, Status);
	return Status;
}
      
NTSTATUS
FsFilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    return STATUS_SUCCESS;
}


VOID
FsFilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
}


VOID
FsFilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
}
      
      
      
      