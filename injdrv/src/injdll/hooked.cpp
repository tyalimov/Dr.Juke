#include "Hooked.h"
#include "mw_tricks.h"

using namespace mwtricks;

extern MalwareTrickChain* g_mw_tricks;

template <typename TFunc, typename TRetVal, typename ...TArgs>
TRetVal HookHandler(const wchar_t* f_name, TFunc f, TArgs&... args)
{
	FunctionCall call_pre(f_name, true, NULL, args...);
	g_mw_tricks->updateCurrentStage(call_pre);

	TRetVal ret_val = f(args...);

	FunctionCall call_post(f_name, false, ret_val,  args...);
	g_mw_tricks->updateCurrentStage(call_post);
	ret_val = call_post.getReturnValue();
	return ret_val;
}

//------------------------------------------------------------//
//                ntdll.dll hooked functions                  //
//------------------------------------------------------------//

NTSTATUS
NTAPI
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
)
{
	return HookHandler<decltype(Orig_NtCreateThreadEx), NTSTATUS>(
		L"ntdll.dll!NtCreateThreadEx",
		Orig_NtCreateThreadEx,
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
}

NTSTATUS
NTAPI
Hooked_NtCreateFile(
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
	return HookHandler<decltype(Orig_NtCreateFile), NTSTATUS>(
		L"ntdll.dll!NtCreateFile",
		Orig_NtCreateFile,
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
Hooked_NtCreateUserProcess(
	_Out_ PHANDLE ProcessHandle,
	_Out_ PHANDLE ThreadHandle,
	_In_ ACCESS_MASK ProcessDesiredAccess,
	_In_ ACCESS_MASK ThreadDesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ProcessObjectAttributes,
	_In_opt_ POBJECT_ATTRIBUTES ThreadObjectAttributes,
	_In_ ULONG ProcessFlags, // PROCESS_CREATE_FLAGS_*
	_In_ ULONG ThreadFlags, // THREAD_CREATE_FLAGS_*
	_In_opt_ PVOID ProcessParameters, // PRTL_USER_PROCESS_PARAMETERS
	_Inout_ PPS_CREATE_INFO CreateInfo,
	_In_opt_ PPS_ATTRIBUTE_LIST AttributeList
)
{
	return HookHandler<decltype(Orig_NtCreateUserProcess), NTSTATUS>(
		L"ntdll.dll!NtCreateUserProcess",
		Orig_NtCreateUserProcess,
		ProcessHandle,
		ThreadHandle,
		ProcessDesiredAccess,
		ThreadDesiredAccess,
		ProcessObjectAttributes,
		ThreadObjectAttributes,
		ProcessFlags, // PROCESS_CREATE_FLAGS_*
		ThreadFlags, // THREAD_CREATE_FLAGS_*
		ProcessParameters, // PRTL_USER_PROCESS_PARAMETERS
		CreateInfo,
		AttributeList
		);
}

NTSTATUS
NTAPI
Hooked_LdrLoadDll(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
)
{
	return HookHandler<decltype(Orig_LdrLoadDll), NTSTATUS>(
		L"ntdll.dll!LdrLoadDll",
		Orig_LdrLoadDll,
		DllPath,
		DllCharacteristics,
		DllName,
		DllHandle
		);
}

NTSTATUS
NTAPI
Hooked_LdrGetDllHandle(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
)
{
	return HookHandler<decltype(Orig_LdrGetDllHandle), NTSTATUS>(
		L"ntdll.dll!LdrGetDllHandle",
		Orig_LdrGetDllHandle,
		DllPath,
		DllCharacteristics,
		DllName,
		DllHandle
		);
}

//------------------------------------------------------------//
//                   ws2_32 hooked functions                  //
//------------------------------------------------------------//


int WSAAPI Hooked_connect(
	SOCKET         s,
	const sockaddr* name,
	int            namelen)
{
	return HookHandler<decltype(Orig_connect), int>(
		L"ws2_32.dll!connect",
		Orig_connect,
		s, name, namelen);
}

int WSAAPI Hooked_recv(
	SOCKET s,
	char* buf,
	int    len,
	int    flags)
{
	return HookHandler<decltype(Orig_recv), int>(
		L"ws2_32.dll!recv",
		Orig_recv,
		s, buf, len, flags);
}

SOCKET WSAAPI Hooked_socket(
	int af,
	int type,
	int protocol)
{
	return HookHandler<decltype(Orig_socket), SOCKET>(
		L"ws2_32.dll!socket",
		Orig_socket,
		af, type, protocol);
}

int WSAAPI Hooked_closesocket(IN SOCKET s)
{
	return HookHandler<decltype(Orig_closesocket), int>(
		L"ws2_32.dll!closesocket",
		Orig_closesocket, s);
}
