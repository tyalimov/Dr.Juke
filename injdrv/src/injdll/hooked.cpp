#include "hooked.h"
#include "mw_tricks.h"

using namespace mwtricks;

extern MalwareTrickChain* g_mw_tricks;

template <typename TFunc, typename TRetVal, typename ...TArgs>
TRetVal HookHandler(const wchar_t* f_name, TFunc f, TArgs&... args)
{
	TRetVal ret_val;
	FunctionCall call_pre(f_name, true, NULL, args...);

	g_mw_tricks->updateCurrentStage(call_pre);
	ret_val = f(args...);

	FunctionCall call_post(f_name, false, NULL,  args...);
	g_mw_tricks->updateCurrentStage(call_post);
	return ret_val;
}

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
	return HookHandler<decltype(OrigNtCreateThreadEx), NTSTATUS>(
		L"NtCreateThreadEx",
		OrigNtCreateThreadEx,
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
	return HookHandler<decltype(OrigNtCreateFile), NTSTATUS>(
		L"NtCreateFile",
		OrigNtCreateFile,
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
HookedNtCreateUserProcess(
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
	return HookHandler<decltype(OrigNtCreateUserProcess), NTSTATUS>(
		L"NtCreateUserProcess",
		OrigNtCreateUserProcess,
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
HookedLdrLoadDll(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
)
{
	return HookHandler<decltype(OrigLdrLoadDll), NTSTATUS>(
		L"LdrLoadDll",
		OrigLdrLoadDll,
		DllPath,
		DllCharacteristics,
		DllName,
		DllHandle
		);
}

NTSTATUS
NTAPI
HookedLdrGetDllHandle(
	_In_opt_ PWSTR DllPath,
	_In_opt_ PULONG DllCharacteristics,
	_In_ PUNICODE_STRING DllName,
	_Out_ PVOID* DllHandle
)
{
	return HookHandler<decltype(OrigLdrGetDllHandle), NTSTATUS>(
		L"LdrGetDllHandle",
		OrigLdrGetDllHandle,
		DllPath,
		DllCharacteristics,
		DllName,
		DllHandle
		);
}
