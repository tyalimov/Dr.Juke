#include "hook_handler.h"
#include "util.h"
#include "trace.h"

api_call_t HookHandlerUpper = nullptr;
HANDLE hLevelTwo = nullptr;

decltype(LoadLibraryExW)* pLoadLibraryExW = nullptr;
decltype(GetProcAddress)* pGetProcAddress = nullptr;

extern decltype(LdrGetDllHandle)* Orig_LdrGetDllHandle;

#pragma warning( disable : 4311 )
#pragma warning( disable : 4302 )

VOID NTAPI
on_LdrLoadDll(ApiCall* call);

VOID NTAPI
on_LdrUnloadDll(ApiCall* call);

VOID NTAPI
HookHandlerLower(ApiCall* call)
{
	switch (call->getCallId())
	{
	case CallId::ntdll_LdrLoadDll:
		on_LdrLoadDll(call);
		break;
	default:
		break;
	}

}

NTSTATUS LoadLevelTwo(PKEY_VALUE_FULL_INFORMATION info)
{
	const wchar_t* path = (const wchar_t*)((PCHAR)info + info->DataOffset);
	hLevelTwo = INVALID_HANDLE_VALUE;

	HMODULE hMod = pLoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	HookHandlerUpper = (api_call_t)pGetProcAddress(hMod, "onApiCall");

	if (HookHandlerUpper != NULL)
	{
		UNICODE_STRING DllName;

#ifdef _M_IX86
		RtlInitUnicodeString(&DllName, (PWSTR)L"injdll2x86.dll");
#else
		RtlInitUnicodeString(&DllName, (PWSTR)L"injdll2x64.dll");
#endif

		Orig_LdrGetDllHandle(NULL, 0 , &DllName, (PVOID*)&hLevelTwo);
		ProtectDll(hLevelTwo);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}

VOID NTAPI
on_LdrLoadDll(ApiCall* call)
{
	if (call->isPost() && NT_SUCCESS(call->getReturnValue()))
	{
		if (hLevelTwo != NULL)
			return;

		PUNICODE_STRING DllName = (PUNICODE_STRING)call->getArgument(2);
		PVOID* pDllHandle = (PVOID *)call->getArgument(3);
		PVOID DllHandle = pDllHandle ? *pDllHandle : NULL;

		if (DllHandle == NULL)
			return;

		UNICODE_STRING NeededDll;
		RtlInitUnicodeString(&NeededDll, (PWSTR)L"kernel32");

		BOOLEAN bkernel32 = RtlPrefixUnicodeString(&NeededDll, DllName, TRUE);
		if (bkernel32)
		{
			ANSI_STRING RoutineName;

			RtlInitAnsiString(&RoutineName, (PSTR)"LoadLibraryExW");
			LdrGetProcedureAddress(DllHandle, &RoutineName, 0, (PVOID*)&pLoadLibraryExW);

			RtlInitAnsiString(&RoutineName, (PSTR)"GetProcAddress");
			LdrGetProcedureAddress(DllHandle, &RoutineName, 0, (PVOID*)&pGetProcAddress);

			RTL_ASSERT(pLoadLibraryExW != NULL);
			RTL_ASSERT(pGetProcAddress != NULL);

			const wchar_t* key = L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\InjDrv";
			const wchar_t* val = L"LevelTwoDll";

			NTSTATUS status = QueryKeyValue(key, val, LoadLevelTwo);
			if (!NT_SUCCESS(status))
				Trace::logError(L"Unable to read LevelTwoDll key - 0x%08X", status);
		}

	}

}
