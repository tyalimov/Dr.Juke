#define _CRT_SECURE_NO_WARNINGS

#include "ref_handles.h"
#include "handle_borne.h"
#include "mw_detect.h"


#define NT_SUCCESS(status) ((NTSTATUS)(status) >= 0)

#pragma warning( disable : 4311 )
#pragma warning( disable : 4302 )

__parent_child__(hProcess, hThread)
__start_func__(MalwareId::ProcessHollowing)
void on_NtCreateUserProcess(ApiCall* call)
{
	if (call->isPre())
		return;

	if (!NT_SUCCESS(call->getReturnValue()))
		return;

	PHANDLE phProcess = (PHANDLE)call->getArgument(0);
	PHANDLE phThread = (PHANDLE)call->getArgument(1);
	HANDLE hProcess = phProcess ? *phProcess : NULL;
	HANDLE hThread = phThread ? *phThread : NULL;

	if (hProcess == NULL || hThread == NULL)
		return;

	HandleBorneEmplace(hProcess, hThread);
	RefHandlesEmplace(CallId::ntdll_NtCreateUserProcess, hProcess);
}

__middle_func__(MalwareId::ProcessHollowing, 2)
void on_NtUnmapViewOfSection(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(0);
	if (!NT_SUCCESS(call->getReturnValue()))
	{
		mwCleanup(hProcess);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtUnmapViewOfSection, hProcess);
	mwDetectProcessHollowing2(hProcess);
}

__middle_func__(MalwareId::ProcessHollowing, 3)
void on_NtWriteVirtualMemory(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(0);
	if (!NT_SUCCESS(call->getReturnValue()))
	{
		mwCleanup(hProcess);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtWriteVirtualMemory, hProcess);
	mwDetectProcessHollowing3(hProcess);
}

__middle_func__(MalwareId::ProcessHollowing, 4)
void on_NtSetContextThread(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hThread = (HANDLE)call->getArgument(0);
	if (!NT_SUCCESS(call->getReturnValue()))
	{
		mwCleanup(hThread);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtSetContextThread, hThread);
	mwDetectProcessHollowing4(hThread);
}

__trigger_func__(MalwareId::ProcessHollowing)
void on_NtResumeThread(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hThread = (HANDLE)call->getArgument(0);
	RefHandlesEmplace(CallId::ntdll_NtResumeThread, hThread);
	mwDetectProcessHollowing(hThread, call);
}

// TODO change geistry path, windows 10
extern "C" __declspec(dllexport) 
void onApiCall(ApiCall* call)
{
	switch (call->getCallId())
	{
	case CallId::ntdll_NtCreateUserProcess:
		on_NtCreateUserProcess(call);
		break;
	case CallId::ntdll_NtUnmapViewOfSection:
		on_NtUnmapViewOfSection(call);
		break;
	case CallId::ntdll_NtWriteVirtualMemory:
		on_NtWriteVirtualMemory(call);
		break;
	case CallId::ntdll_NtSetContextThread:
		on_NtSetContextThread(call);
		break;
	case CallId::ntdll_NtResumeThread:
		on_NtResumeThread(call);
		break;
	default:
		break;
	}
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL,
	DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_PROCESS_DETACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
