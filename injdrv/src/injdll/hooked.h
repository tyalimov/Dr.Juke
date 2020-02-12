#pragma once
//
// Include NTDLL-related headers.
//
#define NTDLL_NO_INLINE_INIT_STRING
#include "hook_handler.h"
#include <winsock2.h>

#define ORIG_DECL(name) \
inline extern decltype(name)* Orig_##name = nullptr;

ORIG_DECL(LdrLoadDll);
ORIG_DECL(LdrGetDllHandle);
ORIG_DECL(LdrUnloadDll);

ORIG_DECL(NtCreateUserProcess);
ORIG_DECL(NtWriteVirtualMemory);
ORIG_DECL(NtUnmapViewOfSection);
ORIG_DECL(NtSetContextThread);
ORIG_DECL(NtResumeThread);
ORIG_DECL(NtOpenProcess);
ORIG_DECL(NtCreateThreadEx);
ORIG_DECL(RtlCreateUserThread);
ORIG_DECL(NtQueueApcThread);
ORIG_DECL(NtSuspendThread);

ORIG_DECL(socket);
ORIG_DECL(closesocket);
ORIG_DECL(connect);
ORIG_DECL(recv);

//------------------------------------------------------------//
//                ntdll.dll hooked functions                  //
//------------------------------------------------------------//

NTSTATUS
NTAPI
Hooked_LdrLoadDll(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
);

NTSTATUS
NTAPI
Hooked_LdrGetDllHandle(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
);

NTSTATUS NTAPI
Hooked_NtCreateUserProcess(
	PHANDLE	ProcessHandle,
	PHANDLE	ThreadHandle,
	ACCESS_MASK	ProcessDesiredAccess,
	ACCESS_MASK	ThreadDesiredAccess,
	POBJECT_ATTRIBUTES ProcessObjectAttributes,
	POBJECT_ATTRIBUTES ThreadObjectAttributes,
	ULONG CreateProcessFlags,
	ULONG CreateThreadFlags,
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
	PPS_CREATE_INFO Unknown,
	PPS_ATTRIBUTE_LIST AttributeList
);

NTSTATUS NTAPI
Hooked_NtWriteVirtualMemory(
	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress,
	_In_reads_bytes_(BufferSize) PVOID Buffer,
	_In_ SIZE_T BufferSize,
	_Out_opt_ PSIZE_T NumberOfBytesWritten
);

NTSTATUS NTAPI
Hooked_NtUnmapViewOfSection(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress
    );

NTSTATUS NTAPI
Hooked_NtSetContextThread(
	_In_ HANDLE ThreadHandle,
	_In_ PCONTEXT ThreadContext
    );

NTSTATUS NTAPI
Hooked_NtResumeThread(
    _In_ HANDLE ThreadHandle,
    _Out_opt_ PULONG PreviousSuspendCount
    );

NTSTATUS NTAPI
Hooked_NtCreateThreadEx(
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

NTSTATUS NTAPI
Hooked_RtlCreateUserThread(
    _In_ HANDLE Process,
    _In_opt_ PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
    _In_ BOOLEAN CreateSuspended,
    _In_opt_ ULONG ZeroBits,
    _In_opt_ SIZE_T MaximumStackSize,
    _In_opt_ SIZE_T CommittedStackSize,
    _In_ PUSER_THREAD_START_ROUTINE StartAddress,
    _In_opt_ PVOID Parameter,
    _Out_opt_ PHANDLE Thread,
    _Out_opt_ PCLIENT_ID ClientId
    );

NTSTATUS NTAPI
Hooked_NtQueueApcThread(
    _In_ HANDLE ThreadHandle,
    _In_ PPS_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcArgument1,
    _In_opt_ PVOID ApcArgument2,
    _In_opt_ PVOID ApcArgument3
    );

NTSTATUS NTAPI
Hooked_NtSuspendThread(
    _In_ HANDLE ThreadHandle,
    _Out_opt_ PULONG PreviousSuspendCount
    );

//------------------------------------------------------------//
//                   ws2_32 hooked functions                  //
//------------------------------------------------------------//

int WSAAPI Hooked_connect(
  SOCKET         s,
  const sockaddr *name,
  int            namelen
);

int WSAAPI Hooked_recv(
  SOCKET s,
  char   *buf,
  int    len,
  int    flags
);

SOCKET WSAAPI Hooked_socket(
  int af,
  int type,
  int protocol
);

int WSAAPI Hooked_closesocket(
  IN SOCKET s
);
