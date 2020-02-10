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

ORIG_DECL(NtCreateUserProcess);
ORIG_DECL(NtWriteVirtualMemory);
ORIG_DECL(NtUnmapViewOfSection);
ORIG_DECL(NtSetContextThread);
ORIG_DECL(NtResumeThread);

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
