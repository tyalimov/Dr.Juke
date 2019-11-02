#pragma once
//
// Include NTDLL-related headers.
//
#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>

#define ORIG_DECL(name) \
inline decltype(name)* Orig##name = nullptr;

ORIG_DECL(NtCreateThreadEx);
ORIG_DECL(NtCreateFile);
ORIG_DECL(NtWriteFile);
ORIG_DECL(NtClose);


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
);

NTSTATUS
NTAPI
HookedNtClose(IN HANDLE ObjectHandle);

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
);

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
);
