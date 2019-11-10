#define _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE 1

#include "wow64log.h"
#include "hooked.h"

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
// Malware filtering
//

#include "mw_tricks.h"
#include "str_util.h"
using namespace ownstl;
using namespace mwtricks;

MalwareTricks* g_mw_tricks = nullptr;

void SetupMalwareFiltering();

//
// Include support for ETW logging.
// Note that following functions are mocked, because they're
// located in advapi32.dll.  Fortunatelly, advapi32.dll simply
// redirects calls to these functions to the ntdll.dll.
//

#define EventActivityIdControl  EtwEventActivityIdControl
#define EventEnabled            EtwEventEnabled
#define EventProviderEnabled    EtwEventProviderEnabled
#define EventRegister           EtwEventRegister
#define EventSetInformation     EtwEventSetInformation
#define EventUnregister         EtwEventUnregister
#define EventWrite              EtwEventWrite
#define EventWriteEndScenario   EtwEventWriteEndScenario
#define EventWriteEx            EtwEventWriteEx
#define EventWriteStartScenario EtwEventWriteStartScenario
#define EventWriteString        EtwEventWriteString
#define EventWriteTransfer      EtwEventWriteTransfer

#include <evntprov.h>

//
// Include Detours.
//

#include <detours.h>

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
	(SIZE_T)& __safe_se_handler_count,
};

#endif

//
// Unfortunatelly sprintf-like functions are not exposed
// by ntdll.lib, which we're linking against.  We have to
// load them dynamically.
//

using _snwprintf_fn_t = int(__cdecl*)(
	wchar_t* buffer,
	size_t count,
	const wchar_t* format,
	...
	);

_snwprintf_fn_t _snwprintf = nullptr;

//
// ETW provider GUID and global provider handle.
//

//
// GUID:
//   {a4b4ba50-a667-43f5-919b-1e52a6d69bd5}
//

GUID ProviderGuid = {
  0xa4b4ba50, 0xa667, 0x43f5, { 0x91, 0x9b, 0x1e, 0x52, 0xa6, 0xd6, 0x9b, 0xd5 }
};

REGHANDLE ProviderHandle;


#define DETOUR_HOOK(func)						\
Orig##func = func;								\
DetourAttach((PVOID*)& Orig##func, Hooked##func)

#define DETOUR_UNHOOK(func)						\
DetourDetach((PVOID*)& Orig##func, Hooked##func)

NTSTATUS
NTAPI
EnableDetours(
	VOID
)
{
	DetourTransactionBegin();
	{
		DETOUR_HOOK(LdrGetDllHandle);
		DETOUR_HOOK(LdrLoadDll);
		DETOUR_HOOK(NtCreateThreadEx);
		DETOUR_HOOK(NtCreateUserProcess);
		DETOUR_HOOK(NtCreateFile);
		DETOUR_HOOK(NtWriteFile);
		DETOUR_HOOK(NtClose);
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
		DETOUR_UNHOOK(NtCreateThreadEx);
		DETOUR_UNHOOK(NtCreateFile);
		DETOUR_UNHOOK(NtWriteFile);
		DETOUR_UNHOOK(NtClose);
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
	//
	// First, resolve address of the _snwprintf function.
	//

	ANSI_STRING RoutineName;
	RtlInitAnsiString(&RoutineName, (PSTR)"_snwprintf");

	UNICODE_STRING NtdllPath;
	RtlInitUnicodeString(&NtdllPath, (PWSTR)L"ntdll.dll");

	HANDLE NtdllHandle;
	LdrGetDllHandle(NULL, 0, &NtdllPath, &NtdllHandle);
	LdrGetProcedureAddress(NtdllHandle, &RoutineName, 0, (PVOID*)& _snwprintf);

	//
	// Make us unloadable (by FreeLibrary calls).
	//

	LdrAddRefDll(LDR_ADDREF_DLL_PIN, ModuleHandle);

	//
	// Hide this DLL from the PEB.
	//

	PPEB Peb = NtCurrentPeb();
	PLIST_ENTRY ListEntry;

	for (ListEntry = Peb->Ldr->InLoadOrderModuleList.Flink;
		ListEntry != &Peb->Ldr->InLoadOrderModuleList;
		ListEntry = ListEntry->Flink)
	{
		PLDR_DATA_TABLE_ENTRY LdrEntry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		//
		// ModuleHandle is same as DLL base address.
		//

		if (LdrEntry->DllBase == ModuleHandle)
		{
			RemoveEntryList(&LdrEntry->InLoadOrderLinks);
			RemoveEntryList(&LdrEntry->InInitializationOrderLinks);
			RemoveEntryList(&LdrEntry->InMemoryOrderLinks);
			RemoveEntryList(&LdrEntry->HashLinks);

			break;
		}
	}

	//
	// Create exports for Wow64Log* functions in
	// the PE header of this DLL.
	//

	Wow64LogCreateExports(ModuleHandle);


	//
	// Register ETW provider.
	//

	EtwEventRegister(&ProviderGuid,
		NULL,
		NULL,
		&ProviderHandle);

	//
	// Get command line of the current process and send it.
	//

	PWSTR CommandLine = Peb->ProcessParameters->CommandLine.Buffer;

	WCHAR Buffer[1024];
	_snwprintf(Buffer,
		RTL_NUMBER_OF(Buffer),
		L"Arch: %s, CommandLine: '%s'",
		ARCH_W,
		CommandLine);

	EtwEventWriteString(ProviderHandle, 0, 0, Buffer);

	//
	// Setup malware filtering rules
	//

	g_mw_tricks = new MalwareTricks();
	SetupMalwareFiltering();

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
	// Cleanup
	//

	delete g_mw_tricks;

	//
	// Unhook all functions.
	//

	return DisableDetours();
}

BOOL NtGetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode)
{
	NTSTATUS status = -1; // eax
	PROCESS_BASIC_INFORMATION ProcessInformation;
	
	status = NtQueryInformationProcess(hProcess, ProcessBasicInformation,
		&ProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), 0i64);

	if (NT_SUCCESS(status))
		*lpExitCode = ProcessInformation.ExitStatus;

	return status;
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

/////////////////////////////////

#define PASSWORD_THEFT		L"[IE Password theft] "
#define INIT				L"[Init] "

PCreateProcessW lpCreateProcessW = nullptr;

wstring GetProcessPid() {
	WCHAR buffer[20] = { 0 };
	UNICODE_STRING us;
	us.Buffer = buffer;
	us.Length = 0;
	us.MaximumLength = 20;
	RtlIntegerToUnicodeString(
		(ULONG)NtCurrentProcessId(), 10, &us);

	return FromUnicodeString(us);
}

wstring GetProcessImagePath() {
	return FromUnicodeString(
		NtCurrentPeb()->ProcessParameters->ImagePathName);
}

wstring GetProcessInformation() {
	return wstring(L"PID=") + GetProcessPid()
		+ L" " + L"PATH=" + GetProcessImagePath();
}

void MT_Theft()
{
	MalwareTrick mt_test(PASSWORD_THEFT L"Via vaultcli.dll");
	mt_test.addCheck([](const FunctionCall& call) {

		if (call.getName() == L"LdrLoadDll")
		{
			PUNICODE_STRING DllName = (PUNICODE_STRING)call.getArgument(2);
			PVOID* DllHandle = (PVOID *)call.getArgument(3);

			if (*DllName == L"kernel32" || *DllName == L"kernel32.dll")
			{
				ANSI_STRING RoutineName;
				RtlInitAnsiString(&RoutineName, (PSTR)"CreateProcessW");
				LdrGetProcedureAddress(*DllHandle, &RoutineName, 0, (PVOID*)&lpCreateProcessW);

				if (lpCreateProcessW == nullptr)
				{
					wstring info = GetProcessInformation() + INIT + L"failed\n";
					EtwEventWriteString(ProviderHandle, 0, 0, info.c_str());
					return false;
				}
				else
				{
					wstring info = GetProcessInformation() + INIT + L"successfull\n";
					EtwEventWriteString(ProviderHandle, 0, 0, info.c_str());
					return true;
				}
			}
		}

		return false;

		});

	mt_test.addCheck([](const FunctionCall& call) {

		if (call.getName() == L"LdrLoadDll")
		{
			PUNICODE_STRING DllName = (PUNICODE_STRING)call.getArgument(2);
			PVOID* DllHandle = (PVOID*)call.getArgument(3);

			if (*DllName == L"vaultcli" || *DllName == L"vaultcli.dll")
			{
				if (lpCreateProcessW != nullptr)
				{
					STARTUPINFOW info = { sizeof(info) };
					PROCESS_INFORMATION processInfo;

					EtwEventWriteString(ProviderHandle, 0, 0, L"Creating signtool process");
					wstring cmd = wstring(L"C:\\signtool.exe verify \"") + GetProcessImagePath() + L"\""; //L"C:\\Windows\\Explorer.exe"; 
					BOOL ok = lpCreateProcessW(NULL, (PWSTR)cmd.c_str(),
						NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);

					if (ok)
					{
						NtWaitForSingleObject(processInfo.hProcess, FALSE, NULL);

						DWORD exit_code;
						NtGetExitCodeProcess(processInfo.hProcess, &exit_code);
						NtClose(processInfo.hProcess);
						NtClose(processInfo.hThread);

						wchar_t buf[64];
						_snwprintf(buf,
							RTL_NUMBER_OF(buf),
							L"Signtool process returned %d",
							exit_code);

						EtwEventWriteString(ProviderHandle, 0, 0, buf);
						if (exit_code != 0)
							return true;
					}
				}
			}
		}

		return false;
		});

	g_mw_tricks->addTrick(mt_test);
}

void SetupMalwareFiltering()
{
	MT_Theft();

	g_mw_tricks->onMalwareDetected([](const wstring& trick_name) {
		wstring ws_info = GetProcessInformation();
		wstring ws_total = ws_info + L"\nTRICK=" + trick_name + L"\n";
		EtwEventWriteString(ProviderHandle, 0, 0, ws_total.c_str());

		if (trick_name.startsWith(PASSWORD_THEFT))
		{
			EtwEventWriteString(ProviderHandle, 0, 0, L"Shutdown process lalala");
			NTSTATUS status = NtSuspendProcess(NtCurrentProcess());
			if (NT_SUCCESS(status))
			{
				EtwEventWriteString(ProviderHandle, 0, 0, L"Success");
			}
			else
			{
				wchar_t buf[64];
				_snwprintf(buf,
					RTL_NUMBER_OF(buf),
					L"Failed, err=%d",
					RtlNtStatusToDosError(status));
			}

		}
		else
			EtwEventWriteString(ProviderHandle, 0, 0, L"Not shutdown process");
	});
}
