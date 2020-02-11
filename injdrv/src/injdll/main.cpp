#define _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE 1

#include "hooked.h"
#include "hook_handler.h"
#include "util.h"
#include "trace.h"

#if defined(_M_IX86)
#  define ARCH_A          "x86"
#  define ARCH_W         L"x86"
#elif defined(_M_AMD64)
#  define ARCH_A          "x64"
#  define ARCH_W         L"x64"
#elif defined(_M_ARM)
#  define ARCH_A          "ARM32"
#  define ARCH_W         L"ARM32"
#elif defined(_M_ARM64)
#  define ARCH_A          "ARM64"
#  define ARCH_W         L"ARM64"
#else
#  error Unknown architecture
#endif

//
// This is necessary for x86 builds because of SEH,
// which is used by Detours.  Look at loadcfg.c file
// in Visual Studio's CRT source codes for the original
// implementation.
//

#if defined(_M_IX86) || defined(_X86_)

EXTERN_C PVOID __safe_se_handler_table[]; /* base of safe handler entry table */
EXTERN_C BYTE  __safe_se_handler_count;   /* absolute symbol whose address is
											 the count of table entries */
EXTERN_C
CONST
DECLSPEC_SELECTANY
IMAGE_LOAD_CONFIG_DIRECTORY
_load_config_used = {
	sizeof(_load_config_used),
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	(SIZE_T)__safe_se_handler_table,
	(SIZE_T)&__safe_se_handler_count,
};

#endif

#include <detours.h>

_snwprintf_fn_t _snwprintf = nullptr;

#define DETOUR_HOOK(func)							\
Orig_##func = func;									\
DetourAttach((PVOID*)& Orig_##func, Hooked_##func)

#define DETOUR_UNHOOK(func)							\
DetourDetach((PVOID*)& Orig_##func, Hooked_##func) 	


NTSTATUS
NTAPI
EnableDetours(
	VOID
)
{
	DetourTransactionBegin();
	{
		DETOUR_HOOK(NtCreateUserProcess);
		DETOUR_HOOK(NtWriteVirtualMemory);
		DETOUR_HOOK(NtUnmapViewOfSection);
		DETOUR_HOOK(NtSetContextThread);
		DETOUR_HOOK(NtResumeThread);
		DETOUR_HOOK(NtCreateThreadEx);
		DETOUR_HOOK(RtlCreateUserThread);

		DETOUR_HOOK(LdrGetDllHandle);
		DETOUR_HOOK(LdrLoadDll);
	}
	DetourTransactionCommit();

	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
DisableDetours(
	VOID
)
{
	DetourTransactionBegin();
	{
		DETOUR_UNHOOK(NtCreateUserProcess);
		DETOUR_UNHOOK(NtWriteVirtualMemory);
		DETOUR_UNHOOK(NtUnmapViewOfSection);
		DETOUR_UNHOOK(NtSetContextThread);
		DETOUR_UNHOOK(NtResumeThread);
		DETOUR_UNHOOK(NtCreateThreadEx);
		DETOUR_UNHOOK(RtlCreateUserThread);

		DETOUR_UNHOOK(LdrGetDllHandle);
		DETOUR_UNHOOK(LdrLoadDll);
	}
	DetourTransactionCommit();
	return STATUS_SUCCESS;
}



NTSTATUS
NTAPI
OnProcessAttach(
	_In_ PVOID ModuleHandle
)
{
	UNICODE_STRING DllName;
	RtlInitUnicodeString(&DllName, (PWSTR)L"ntdll.dll");

	HANDLE NtdllHandle;
	LdrGetDllHandle(NULL, 0, &DllName, &NtdllHandle);

	Trace::init(NtdllHandle);
	ProtectDll(ModuleHandle);

	//
	// Get command line of the current process and send it.
	//

	PPEB Peb = NtCurrentPeb();
	PWSTR CommandLine = Peb->ProcessParameters->CommandLine.Buffer;

	Trace::logInfo(L"Arch: %s, CommandLine: '%s'", ARCH_W, CommandLine);

	//
	// Check kernel32 has been already loaded
	// If so, we are able to load level 2 dll
	//

	HANDLE Kernel32Handle;
	RtlInitUnicodeString(&DllName, (PWSTR)L"kernel32.dll");
	LdrGetDllHandle(NULL, 0, &DllName, &Kernel32Handle);

	if (Kernel32Handle != NULL)
	{
		ApiCall call(CallId::ntdll_LdrLoadDll, false,
			(PWSTR)NULL, (PULONG)0, &DllName, &Kernel32Handle);

		HookHandlerLower(&call);
	}

	//
	// Hook all functions.
	//

	return EnableDetours();
}

NTSTATUS
NTAPI
OnProcessDetach(
	_In_ HANDLE ModuleHandle
)
{
	//
	// Unhook all functions.
	//

	return DisableDetours();
}

EXTERN_C
BOOL
NTAPI
NtDllMain(
	_In_ HANDLE ModuleHandle,
	_In_ ULONG Reason,
	_In_ LPVOID Reserved
)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		OnProcessAttach(ModuleHandle);
		break;

	case DLL_PROCESS_DETACH:
		OnProcessDetach(ModuleHandle);
		break;

	case DLL_THREAD_ATTACH:

		break;

	case DLL_THREAD_DETACH:

		break;
	}

	return TRUE;
}

//wstring GetProcessPid() {
//	WCHAR buffer[20] = { 0 };
//	UNICODE_STRING us;
//	us.Buffer = buffer;
//	us.Length = 0;
//	us.MaximumLength = 20;
//	RtlIntegerToUnicodeString(
//		(ULONG)NtCurrentProcessId(), 10, &us);
//
//	return FromUnicodeString(us);
//}
//
//wstring GetProcessImagePath() {
//	return FromUnicodeString(
//		NtCurrentPeb()->ProcessParameters->ImagePathName);
//}
//
//wstring GetProcessInfo() {
//	return GetProcessPid() + L" | " + GetProcessImagePath();
//}

