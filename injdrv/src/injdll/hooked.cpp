#include "hooked.h"
#include "mw_tricks.h"

using namespace mwtricks;

extern MalwareTricks* g_mw_tricks;

NTSTATUS
NTAPI
HookedNtCreateThreadEx(
	_Out_ PHANDLE ThreadHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ HANDLE ProcessHandle,
	_In_ PVOID StartRoutine, // PUSER_THREAD_START_ROUTINE
	_In_opt_ PVOID Argument,
	_In_ ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
	_In_ SIZE_T ZeroBits,
	_In_ SIZE_T StackSize,
	_In_ SIZE_T MaximumStackSize,
	_In_opt_ PPS_ATTRIBUTE_LIST AttributeList
)
{
	FunctionCall call(
		"NtCreateThreadEx",
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		StartRoutine,
		Argument,
		CreateFlags,
		ZeroBits,
		StackSize,
		MaximumStackSize,
		AttributeList
	);

	g_mw_tricks->updateCurrentStage(call);
	return OrigNtCreateThreadEx(
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		StartRoutine,
		Argument,
		CreateFlags,
		ZeroBits,
		StackSize,
		MaximumStackSize,
		AttributeList);
}

NTSTATUS
NTAPI
HookedNtClose(IN HANDLE ObjectHandle)
{
	FunctionCall call("NtClose", ObjectHandle);
	g_mw_tricks->updateCurrentStage(call);
	return OrigNtClose(ObjectHandle);
}

NTSTATUS
NTAPI
HookedNtCreateFile(
	OUT PHANDLE           FileHandle,
	IN ACCESS_MASK        DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT PIO_STATUS_BLOCK  IoStatusBlock,
	IN PLARGE_INTEGER     AllocationSize,
	IN ULONG              FileAttributes,
	IN ULONG              ShareAccess,
	IN ULONG              CreateDisposition,
	IN ULONG              CreateOptions,
	IN PVOID              EaBuffer,
	IN ULONG              EaLength
)
{
	FunctionCall call(
		"NtCreateFile",
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength
	);
	g_mw_tricks->updateCurrentStage(call);
	return OrigNtCreateFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength
	);
}

NTSTATUS
NTAPI
HookedNtWriteFile(
	HANDLE           FileHandle,
	HANDLE           Event,
	PIO_APC_ROUTINE  ApcRoutine,
	PVOID            ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID            Buffer,
	ULONG            Length,
	PLARGE_INTEGER   ByteOffset,
	PULONG           Key
)
{
	FunctionCall call(
		"NtWriteFile",
		FileHandle,
		Event,
		ApcRoutine,
		ApcContext,
		IoStatusBlock,
		Buffer,
		Length,
		ByteOffset,
		Key
	);

	g_mw_tricks->updateCurrentStage(call);
	return OrigNtWriteFile(
		FileHandle,
		Event,
		ApcRoutine,
		ApcContext,
		IoStatusBlock,
		Buffer,
		Length,
		ByteOffset,
		Key
	);
}
