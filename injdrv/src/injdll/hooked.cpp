#include "hooked.h"
#include "trace.h"
#include "util.h"

extern api_call_t HookHandlerUpper;

#pragma warning( disable : 4312 )
#pragma warning( disable : 4311 )
#pragma warning( disable : 4302 )

template <typename TRetVal, typename TFunc, typename ...TArgs>
TRetVal HookHandler(CallId id, TFunc f, TArgs&... args)
{
	PUNICODE_STRING path = CurrentProcessImagePath();

	//
	// Call pre handlers
	//

	ApiCall call_pre(id, true, args...);
	HookHandlerLower(&call_pre);

	if (HookHandlerUpper != NULL)
		HookHandlerUpper(&call_pre, path->Buffer, path->Length);

	//
	// Check for malware
	//

	MalwareId mid = call_pre.getMalwareId();
	if (mid != MalwareId::None)
	{
		const wchar_t fmt[] = L"Malware detected - <Type=%ws ImagePath=%wZ>";

		switch (mid)
		{
		case MalwareId::ProcessHollowing:
			Trace::logWarning(fmt, L"ProcessHollowing", path);
			break;
		case MalwareId::SimpleProcessInjection:
			Trace::logWarning(fmt, L"SimpleProcessInjection", path);
			break;
		case MalwareId::ThreadHijacking:
			Trace::logWarning(fmt, L"ThreadHijacking", path);
			break;
		case MalwareId::EarlyBird:
			Trace::logWarning(fmt, L"EarlyBird", path);
			break;
		case MalwareId::ApcInjection:
			Trace::logWarning(fmt, L"ApcInjection", path);
			break;
		default:
			break;
		}

		NtSuspendProcess(NtCurrentProcess());
	}
	
	if (call_pre.isCallSkipped())
		return (TRetVal)call_pre.getReturnValue();

	//
	// Call post handlers
	//

	TRetVal ret_val = f(args...);
	ApiCall call_post(id, false, args...);

	call_post.setReturnValue((ApiCall::arg_t)ret_val);
	HookHandlerLower(&call_post);

	if (HookHandlerUpper != NULL)
		HookHandlerUpper(&call_post, path->Buffer, path->Length);

	return (TRetVal)call_post.getReturnValue();
}

//------------------------------------------------------------//
//                ntdll.dll hooked functions                  //
//------------------------------------------------------------//

NTSTATUS NTAPI
Hooked_LdrLoadDll(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
)
{
	return HookHandler<NTSTATUS, decltype(LdrLoadDll)>(
		CallId::ntdll_LdrLoadDll,
		Orig_LdrLoadDll,
		DllPath,
		DllCharacteristics,
		DllName,
		DllHandle
		);
}

NTSTATUS NTAPI
Hooked_LdrGetDllHandle(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
)
{
	return HookHandler<NTSTATUS, decltype(LdrGetDllHandle)>(
		CallId::ntdll_LdrGetDllHandle,
		Orig_LdrGetDllHandle,
		DllPath,
		DllCharacteristics,
		DllName,
		DllHandle
		);
}

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
)
{
	return HookHandler<NTSTATUS, decltype(NtCreateUserProcess)>(
		CallId::ntdll_NtCreateUserProcess,
		Orig_NtCreateUserProcess,
		ProcessHandle,
		ThreadHandle,
		ProcessDesiredAccess,
		ThreadDesiredAccess,
		ProcessObjectAttributes,
		ThreadObjectAttributes,
		CreateProcessFlags,
		CreateThreadFlags,
		ProcessParameters,
		Unknown,
		AttributeList
		);
}

NTSTATUS NTAPI
Hooked_NtWriteVirtualMemory(
	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress,
	_In_reads_bytes_(BufferSize) PVOID Buffer,
	_In_ SIZE_T BufferSize,
	_Out_opt_ PSIZE_T NumberOfBytesWritten
)
{
	return HookHandler<NTSTATUS, decltype(NtWriteVirtualMemory)>(
		CallId::ntdll_NtWriteVirtualMemory,
		Orig_NtWriteVirtualMemory,
		ProcessHandle,
		BaseAddress,
		Buffer,
		BufferSize,
		NumberOfBytesWritten
		);
}

NTSTATUS NTAPI
Hooked_NtUnmapViewOfSection(
	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress
)
{
	return HookHandler<NTSTATUS, decltype(NtUnmapViewOfSection)>(
		CallId::ntdll_NtUnmapViewOfSection,
		Orig_NtUnmapViewOfSection,
		ProcessHandle,
		BaseAddress
		);
}

NTSTATUS NTAPI
Hooked_NtSetContextThread(
	_In_ HANDLE ThreadHandle,
	_In_ PCONTEXT ThreadContext
)
{
	return HookHandler<NTSTATUS, decltype(NtSetContextThread)>(
		CallId::ntdll_NtSetContextThread,
		Orig_NtSetContextThread,
		ThreadHandle,
		ThreadContext
		);
}

NTSTATUS NTAPI
Hooked_NtResumeThread(
	_In_ HANDLE ThreadHandle,
	_Out_opt_ PULONG PreviousSuspendCount
)
{
	return HookHandler<NTSTATUS, decltype(NtResumeThread)>(
		CallId::ntdll_NtResumeThread,
		Orig_NtResumeThread,
		ThreadHandle,
		PreviousSuspendCount
		);
}

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
)
{
	return HookHandler<NTSTATUS, decltype(NtCreateThreadEx)>(
		CallId::ntdll_NtCreateThreadEx,
		Orig_NtCreateThreadEx,
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		StartRoutine, // PUSER_THREAD_START_ROUTINE
		Argument,
		CreateFlags, // THREAD_CREATE_FLAGS_*
		ZeroBits,
		StackSize,
		MaximumStackSize,
		AttributeList
		);
}

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
)
{
	return HookHandler<NTSTATUS, decltype(RtlCreateUserThread)>(
		CallId::ntdll_RtlCreateUserThread,
		Orig_RtlCreateUserThread,
		Process,
		ThreadSecurityDescriptor,
		CreateSuspended,
		ZeroBits,
		MaximumStackSize,
		CommittedStackSize,
		StartAddress,
		Parameter,
		Thread,
		ClientId
		);
}

NTSTATUS NTAPI
Hooked_NtQueueApcThread(
	_In_ HANDLE ThreadHandle,
	_In_ PPS_APC_ROUTINE ApcRoutine,
	_In_opt_ PVOID ApcArgument1,
	_In_opt_ PVOID ApcArgument2,
	_In_opt_ PVOID ApcArgument3
)
{
	return HookHandler<NTSTATUS, decltype(NtQueueApcThread)>(
		CallId::ntdll_NtQueueApcThread,
		Orig_NtQueueApcThread,
		ThreadHandle,
		ApcRoutine,
		ApcArgument1,
		ApcArgument2,
		ApcArgument3
		);
}

NTSTATUS NTAPI
Hooked_NtSuspendThread(
	_In_ HANDLE ThreadHandle,
	_Out_opt_ PULONG PreviousSuspendCount
)
{
	return HookHandler<NTSTATUS, decltype(NtSuspendThread)>(
		CallId::ntdll_NtSuspendThread,
		Orig_NtSuspendThread,
		ThreadHandle,
		PreviousSuspendCount
		);
}

//------------------------------------------------------------//
//                   ws2_32 hooked functions                  //
//------------------------------------------------------------//


//int WSAAPI Hooked_connect(
//	SOCKET         s,
//	const sockaddr* name,
//	int            namelen)
//{
//	return HookHandler<decltype(Orig_connect), int>(
//		CallId::ws2_32_connect,
//		Orig_connect,
//		s, name, namelen);
//}
//
//int WSAAPI Hooked_recv(
//	SOCKET s,
//	char* buf,
//	int    len,
//	int    flags)
//{
//	return HookHandler<decltype(Orig_recv), int>(
//		CallId::ws2_32_recv,
//		Orig_recv,
//		s, buf, len, flags);
//}
//
//SOCKET WSAAPI Hooked_socket(
//	int af,
//	int type,
//	int protocol)
//{
//	return HookHandler<decltype(Orig_socket), SOCKET>(
//		CallId::ws2_32_socket,
//		Orig_socket,
//		af, type, protocol);
//}
//
//int WSAAPI Hooked_closesocket(IN SOCKET s)
//{
//	return HookHandler<decltype(Orig_closesocket), int>(
//		CallId::ws2_32_closesocket,
//		Orig_closesocket, s);
//}
