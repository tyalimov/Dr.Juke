#include "hooked.h"

extern api_call_t HookHandlerUpper;

template <typename TFunc, typename TRetVal, typename ...TArgs>
TRetVal HookHandler(CallId id, TFunc f, TArgs&... args)
{
	//
	// Call pre handlers
	//

	ApiCall call_pre(id, true, args...);
	HookHandlerLower(&call_pre);

	if (HookHandlerUpper != NULL)
		HookHandlerUpper(&call_pre);

	if (call_pre.isCallSkipped())
		return call_pre.getReturnValue();

	//
	// Call post handlers
	//

	TRetVal ret_val = f(args...);
	ApiCall call_post(id, false, args...);

	call_post.setReturnValue(ret_val);
	HookHandlerLower(&call_post);

	if (HookHandlerUpper != NULL)
		HookHandlerUpper(&call_post);

	return call_post.getReturnValue();
}

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
)
{
	return HookHandler<decltype(Orig_LdrLoadDll), NTSTATUS>(
		CallId::ntdll_LdrLoadDll,
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
		CallId::ntdll_LdrGetDllHandle,
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
		CallId::ws2_32_connect,
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
		CallId::ws2_32_recv,
		Orig_recv,
		s, buf, len, flags);
}

SOCKET WSAAPI Hooked_socket(
	int af,
	int type,
	int protocol)
{
	return HookHandler<decltype(Orig_socket), SOCKET>(
		CallId::ws2_32_socket,
		Orig_socket,
		af, type, protocol);
}

int WSAAPI Hooked_closesocket(IN SOCKET s)
{
	return HookHandler<decltype(Orig_closesocket), int>(
		CallId::ws2_32_closesocket,
		Orig_closesocket, s);
}
