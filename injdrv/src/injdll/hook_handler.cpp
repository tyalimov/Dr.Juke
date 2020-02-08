#include "hook_handler.h"
#include "util.h"
#include "trace.h"

api_call_t HookHandlerUpper = nullptr;

decltype(LoadLibraryExW)* pLoadLibraryExW = nullptr;
decltype(GetProcAddress)* pGetProcAddress = nullptr;


VOID NTAPI
on_LdrLoadDll(ApiCall* call);

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
	HMODULE hLevelTwo;
	const wchar_t* path;

	path = (const wchar_t*)((PCHAR)info + info->DataOffset);
	hLevelTwo = pLoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	HookHandlerUpper = (api_call_t)pGetProcAddress(hLevelTwo, "onApiCall");

	if (HookHandlerUpper != NULL)
		return STATUS_SUCCESS;
	else
		return STATUS_UNSUCCESSFUL;
}

VOID NTAPI
on_LdrLoadDll(ApiCall* call)
{
	if (call->isPost() && NT_SUCCESS(call->getReturnValue()))
	{
		if (HookHandlerUpper != NULL)
			return;

		PUNICODE_STRING DllName = (PUNICODE_STRING)call->getArgument(2);
		PVOID* DllHandle = (PVOID *)call->getArgument(3);

		UNICODE_STRING NeededDll;
		RtlInitUnicodeString(&NeededDll, (PWSTR)L"kernel32");

		BOOLEAN bkernel32 = RtlPrefixUnicodeString(&NeededDll, DllName, TRUE);
		if (bkernel32)
		{
			ANSI_STRING VarRoutineName;
			GET_PROC_ADDR("LoadLibraryExW", *DllHandle, pLoadLibraryExW, VarRoutineName);
			GET_PROC_ADDR("GetProcAddress", *DllHandle, pGetProcAddress, VarRoutineName);
			RTL_ASSERT(pLoadLibraryExW != NULL);
			RTL_ASSERT(pGetProcAddress != NULL);

#ifdef _M_IX86
			const wchar_t* key = L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\InjDrv\\x86";
#else
			const wchar_t* key = L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\InjDrv\\x64";
#endif // _M_IX86

			const wchar_t* val = L"LevelTwoDll";

			NTSTATUS status = QueryKeyValue(key, val, LoadLevelTwo);
			if (!NT_SUCCESS(status))
				Trace::logError(L"Unable to read LevelTwoDll key - 0x%08X", status);
		}

	}

}
