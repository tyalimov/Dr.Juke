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

inline decltype(NtCreateThreadEx)* OrigNtCreateThreadEx = nullptr;

NTSTATUS
NTAPI
EnableDetours(
	VOID
)
{
	DetourTransactionBegin();
	{
		OrigNtCreateThreadEx = NtCreateThreadEx;
		DetourAttach((PVOID*)&OrigNtCreateThreadEx, HookNtCreateThreadEx);
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
		DetourDetach((PVOID*)& OrigNtCreateThreadEx, HookNtCreateThreadEx);
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

#define DEFENSE_EVASION		"[Defense Evasion]"
#define TEST				"[Test]"

void MT_Test()
{
	MalwareTrick mt_test("[Test] MalwareTrick");
	mt_test.addCheck([](const FunctionCall& call) {

		if (call.getName() == "NtCreateFile")
		{
			POBJECT_ATTRIBUTES attr = (POBJECT_ATTRIBUTES)call.getArgument(2);
			return (call.getArgument(1) & FILE_WRITE_ACCESS)
				&& *attr->ObjectName == L"test.txt";
		}

		return false;
		});

	mt_test.addCheck([](const FunctionCall& call) {

		if (call.getName() == "NtWriteFile")
		{
			ULONG length = call.getArgument(5);
			string buffer = (const char*)call.getArgument(6);

			return (buffer == "This is test")
				&& length == 12;
		}

		return false;
		});

	mt_test.addCheck([](const FunctionCall& call) {

		if (call.getName() == "NtCloseFile")
			return true;

		return false;
		});

	g_mw_tricks->addNewTrick(mt_test);
}

string GetProcessInformation()
{
	//UNICODE_STRING file_name;
	//DWORD dwSizeNeeded = 0;
	//
	//NtCurrentProcessId();
	//
	//DWORD status = NtQueryInformationProcess(
	//	NtCurrentProcess(), ProcessImageFileName,
	//	&file_name, sizeof(UNICODE_STRING), &dwSizeNeeded);
	//
	//if (NT_SUCCESS(status))
	//{
	//	// Basic Info
	//
	//	// pid, (DWORD)pbi.UniqueProcessId;
	//	// parent pid (DWORD)pbi.InheritedFromUniqueProcessId;
	//	
	//}
	

	//GetModuleFileNameA()
	return"";
}

void SetupMalwareFiltering()
{
	GetProcessInformation();

	MT_Test();

	g_mw_tricks->onMalwareDetected([](const string& trick_name) {

		if (trick_name.startsWith(DEFENSE_EVASION))
		{

		}

		//WCHAR Buffer[1024];
		//_snwprintf(Buffer,
		//	RTL_NUMBER_OF(Buffer),
		//	L"Arch: %s, CommandLine: '%s'",
		//	ARCH_W,
		//	CommandLine);

		//EtwEventWriteString(ProviderHandle, 0, 0, Buffer);

	});
}
