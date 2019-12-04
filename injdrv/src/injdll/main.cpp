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
#include "net_filter.h"

using namespace ownstl;
using namespace mwtricks;

MalwareTrickChain* g_mw_tricks = nullptr;

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
	(SIZE_T)&__safe_se_handler_count,
};

#endif

//
// Unfortunatelly sprintf-like functions are not exposed
// by ntdll.lib, which we're linking against.  We have to
// load them dynamically.
//

#pragma region functions_to_import

_snwprintf_fn_t _snwprintf = nullptr;
_tolower_t __tolower = nullptr;
_towlower_t __towlower = nullptr;
_inet_ntop_t _inet_ntop = nullptr;
_ntohs_t _ntohs = nullptr;

#pragma endregion functions_to_import

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

#define GET_PROC_ADDR(FuncName, DllHandle, FuncAddr, VarRoutineName)					\
RtlInitAnsiString(&VarRoutineName, (PSTR)FuncName);							\
LdrGetProcedureAddress(DllHandle, &VarRoutineName, 0, (PVOID*)& FuncAddr);	\
RTL_ASSERT(FuncAddr != nullptr);

#define DETOUR_HOOK(func)							\
Orig_##func = func;									\
DetourAttach((PVOID*)& Orig_##func, Hooked_##func)

#define DETOUR_UNHOOK(func)							\
DetourDetach((PVOID*)& Orig_##func, Hooked_##func) 	

#define DETOUR_HOOK_ON_DLL_LOAD(func, DllHandle, VarRoutineName)				\
RtlInitAnsiString(&VarRoutineName, (PSTR)#func);							\
LdrGetProcedureAddress(DllHandle, &VarRoutineName, 0, (PVOID*)& Orig_##func);\
RTL_ASSERT(Orig_##func != nullptr); \
DetourAttach((PVOID*)& Orig_##func, Hooked_##func)




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
		DETOUR_UNHOOK(LdrGetDllHandle);
		DETOUR_UNHOOK(LdrLoadDll);
		DETOUR_UNHOOK(NtCreateThreadEx);
		DETOUR_UNHOOK(NtCreateUserProcess);
		DETOUR_UNHOOK(NtCreateFile);
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

	UNICODE_STRING NtdllPath;
	RtlInitUnicodeString(&NtdllPath, (PWSTR)L"ntdll.dll");

	HANDLE NtdllHandle;
	LdrGetDllHandle(NULL, 0, &NtdllPath, &NtdllHandle);

	ANSI_STRING RoutineName;
	GET_PROC_ADDR("_snwprintf", NtdllHandle, _snwprintf, RoutineName);
	GET_PROC_ADDR("tolower", NtdllHandle, __tolower, RoutineName);
	GET_PROC_ADDR("towlower", NtdllHandle, __towlower, RoutineName);

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

	g_mw_tricks = new MalwareTrickChain();
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

//------------------------------------------------------------//
//           Malware filtering rules                          //
//------------------------------------------------------------//

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

wstring GetProcessInfo() {
	return GetProcessPid() + L" | " + GetProcessImagePath();
}

#define TEST L"[TEST] "
#define CONSOLE_INPUT L"[CONSOLE INPUT] "

#define NETFILTER L"NetFilter"
#define WS2_32 "ws2_32"
#define NETFILTER_WS2_32 L"[" NETFILTER L"::" WS2_32 L"]"

void MT_Test()
{
	MalwareTrick mt_test(TEST);
	mt_test.addCheck([](const FunctionCall& call) {

		if (call.getName().endsWith(L"LdrLoadDll"))
		{
			PUNICODE_STRING DllName = (PUNICODE_STRING)call.getArgument(2);
			wstring name = !DllName ? L"<null>" : FromUnicodeString(DllName);
			PVOID* pDllHandle = (PVOID*)call.getArgument(3);
			PVOID DllHandle = !pDllHandle ? NULL : *pDllHandle;

			wchar_t buf[128] = { 0 };

			if (call.isPre())
				_snwprintf(buf, sizeof(buf), L"Before Loading %s\n", name);
			else
				_snwprintf(buf, sizeof(buf), L"After Loading %s. Handle=0x%08X\n", name.c_str(), DllHandle);

			EtwEventWriteString(ProviderHandle, 0, 0, buf);
		}

		return false;

		});

	g_mw_tricks->addTrick(mt_test);
}

void MT_DllUnload()
{
	MalwareTrick mt_dll_unload(TEST);
	mt_dll_unload.addCheck([](const FunctionCall& call) {

		if (call.getName().endsWith(L"LdrUnloadDll") && call.isPre())
		{
			PUNICODE_STRING DllName = (PUNICODE_STRING)call.getArgument(2);
			wstring DllNameStr = !DllName ? L"<null>" : FromUnicodeString(DllName);
			if (DllNameStr == L"ws2_32.dll" || DllNameStr == L"ws2_32")
			{
				DetourTransactionBegin();
				{
					DETOUR_UNHOOK(socket);
					DETOUR_UNHOOK(closesocket);
					DETOUR_UNHOOK(connect);
					DETOUR_UNHOOK(recv);
				}
				DetourTransactionCommit();
			}
		}

		return false;
		});

	g_mw_tricks->addTrick(mt_dll_unload);
}

void MT_NetFilterWS2_32()
{
	MalwareTrick mt_ws2_32(TEST);
	mt_ws2_32.addCheck([](const FunctionCall& call) {

		if (call.getName().endsWith(L"LdrLoadDll") && call.isPost())
		{
			PUNICODE_STRING DllName = (PUNICODE_STRING)call.getArgument(2);
			wstring DllNameStr = !DllName ? L"<null>" : FromUnicodeString(DllName);
			PVOID* pDllHandle = (PVOID*)call.getArgument(3);
			PVOID DllHandle = !pDllHandle ? NULL : *pDllHandle;

			if (NT_SUCCESS(call.getReturnValue()) && DllHandle)
			{
				LPVOID proc_addr = NULL;
				if (DllNameStr == L"ws2_32.dll" || DllNameStr == L"ws2_32")
				{
					DetourTransactionBegin();
					{
						ANSI_STRING RoutineName;
						GET_PROC_ADDR("inet_ntop", DllHandle, _inet_ntop, RoutineName);
						GET_PROC_ADDR("ntohs", DllHandle, _ntohs, RoutineName);
						DETOUR_HOOK_ON_DLL_LOAD(socket, DllHandle, RoutineName);
						DETOUR_HOOK_ON_DLL_LOAD(closesocket, DllHandle, RoutineName);
						DETOUR_HOOK_ON_DLL_LOAD(connect, DllHandle, RoutineName);
						DETOUR_HOOK_ON_DLL_LOAD(recv, DllHandle, RoutineName);
					}
					DetourTransactionCommit();
					EtwEventWriteString(ProviderHandle, 0, 0, L"Loaded ws2_32.dll");
					return true;
				}
			}

		}

		return false;

		});

	mt_ws2_32.addCheck([](const FunctionCall& call) {

		wstring call_name = call.getName();
		if (call_name.startsWith(L"ws2_32.dll"))
		{
			if (call_name.endsWith(L"closesocket") && call.isPost())
			{
				SOCKET s = (SOCKET)call.getArgument(0);
				netfilter::ws2_32::on_closesocket(s);
			}

			else if (call_name.endsWith(L"socket") && call.isPost())
			{
				SOCKET s = call.getReturnValue();
				if (s != INVALID_SOCKET)
				{
					int af = (int)call.getArgument(0);
					int type = (int)call.getArgument(1);
					int prot = (int)call.getArgument(2);
					netfilter::ws2_32::on_socket(s, af, type, prot);
				}
			}


			else if (call_name.endsWith(L"connect") && call.isPost())
			{
				if (call.getReturnValue() == 0)
				{
					SOCKET s = (SOCKET)call.getArgument(0);
					sockaddr* name = (sockaddr*)call.getArgument(1);
					int addr_len = (int)call.getArgument(2);
					netfilter::ws2_32::on_connect(s, name, addr_len);
				}
			}

			else if (call_name.endsWith(L"recv") && call.isPost())
			{
				int flags = (int)call.getArgument(3);
				if (flags == 0)
				{
					SOCKET s = (SOCKET)call.getArgument(0);
					char* buf = (char*)call.getArgument(1);
					int bytes_read = call.getReturnValue();

					netfilter::ws2_32::on_recv(s, buf, bytes_read);
				}
			}
		}

		return false;

		});

	g_mw_tricks->addTrick(mt_ws2_32);
}


void SetupMalwareFiltering()
{
	//MT_Test();
	MT_DllUnload();
	MT_NetFilterWS2_32();


	g_mw_tricks->onMalwareDetected([](const wstring& trick_name) {
		wstring ws_info = GetProcessInfo();
		wstring ws_total = ws_info + L" | " + trick_name + L"\n";
		EtwEventWriteString(ProviderHandle, 0, 0, ws_total.c_str());

		//	if (trick_name.startsWith(NETFILTER))
		//	{
		//		if (trick_name.endsWith(WS2_32))
		//		{
		//			EtwEventWriteString(ProviderHandle, 0, 0, NETFILTER_WS2_32);
		//			netfilter::ws2_32::get_bad_sock_info()
		//		}
		//	}

			//if (trick_name.startsWith(PASSWORD_THEFT))
			//{
			//	EtwEventWriteString(ProviderHandle, 0, 0, L"Shutdown process");
			//	NTSTATUS status = NtSuspendProcess(NtCurrentProcess());
			//	if (NT_ERROR(status))
			//	{
			//		wchar_t buf[64];
			//		_snwprintf(buf,
			//			RTL_NUMBER_OF(buf),
			//			L"Failed, err=%d",
			//			RtlNtStatusToDosError(status));
			//	}
			//}
		});
}

__declspec(dllexport) void f()
{

}
