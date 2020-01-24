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
		kprintf(TRACE_INFO, "Attached to new volume <flags:0x%08x, device:0x%08x, fs:0x%08x>",
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
	PFLT_FILE_NAME_INFORMATION fltName;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	DesiredAccess = Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;
	Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED, &fltName);

	if (!NT_SUCCESS(Status))
	{
		if (Status != STATUS_OBJECT_PATH_NOT_FOUND)
			kprintf(TRACE_INFO, "FltGetFileNameInformation() failed with code:%08x", Status);

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	//kprintf(TRACE_FSFILTER, "%Name=%wZ Component=%wZ", fltName->Name, fltName->FinalComponent);
	FltReleaseFileNameInformation(fltName);

	// if (FALSE)
	// {
	// 	kprintf(TRACE_INFO, "Operation has been cancelled for: %wZ", &Data->Iopb->TargetFileObject->FileName);
	// 	Data->IoStatus.Status = STATUS_NO_SUCH_FILE;
	// 	return FLT_PREOP_COMPLETE;
	// }

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS FsFilterInit(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS Status;
	Status = FltRegisterFilter(DriverObject, &FilterRegistration, &gFilterHandle);

	if (NT_SUCCESS(Status))
	{
		Status = FltStartFiltering(gFilterHandle);
		if (!NT_SUCCESS(Status))
		{
			FltUnregisterFilter(gFilterHandle);
			gFilterHandle = nullptr;
		}
	}

	kprint_st(TRACE_FSFILTER, Status);
	return Status;
}

NTSTATUS FsFilterExit(FLT_FILTER_UNLOAD_FLAGS Flags)
{
	UNREFERENCED_PARAMETER(Flags);

	if (gFilterHandle != nullptr)
	{
		FltUnregisterFilter(gFilterHandle);
		gFilterHandle = nullptr;
	}

	kprint_st(TRACE_FSFILTER, STATUS_SUCCESS);
	return STATUS_SUCCESS;
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
      
      
      
      