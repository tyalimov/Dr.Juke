#include "hooked.h"
#include "mw_tricks.h"

using namespace mwtricks;

extern inline decltype(NtCreateThreadEx)* OrigNtCreateThreadEx = nullptr;

extern MalwareTricks* g_mw_tricks;

NTSTATUS
NTAPI
HookNtCreateThreadEx(
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
		"HookNtCreateThreadEx",
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
