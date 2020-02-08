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
ORIG_DECL(NtReadFile);

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
