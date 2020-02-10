#define _CRT_SECURE_NO_WARNINGS

#include "ref_handles.h"
#include "handle_borne.h"
#include "mw_detect.h"

#include "log.h"

#define NT_SUCCESS(status) ((NTSTATUS)(status) >= 0)


HANDLE GetHandleFromPtr(ApiCall* call, int arg_num)
{
	PHANDLE handle_ptr = (PHANDLE)call->getArgument(arg_num);
	HANDLE handle = nullptr;

	if (handle_ptr != nullptr)
		handle = *handle_ptr;

	return handle;
}

bool CallFilter(ApiCall* call, int handle_arg_num, bool is_pre, PHANDLE out_handle)
{
	*out_handle = nullptr;

	if (call->isPre() != is_pre)
		return false;

	HANDLE call_handle = GetHandleFromPtr(call, handle_arg_num);
	if (call_handle == nullptr)
		return false;

	*out_handle = call_handle;
	return true;
}

__parent_child__(hProcess, hThread)
__start_func__(MalwareId::ProcessHollowing)
void on_NtCreateUserProcess(ApiCall* call)
{
	HANDLE hProcess;
	HANDLE hThread;

	if (!CallFilter(call, 0, false, &hProcess))
		return;

	if (!CallFilter(call, 1, false, &hThread))
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
	RefHandlesEmplace(CallId::ntdll_NtUnmapViewOfSection, hProcess);
	mwDetectProcessHollowing2(hProcess);
}

__middle_func__(MalwareId::ProcessHollowing, 3)
void on_NtWriteVirtualMemory(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(0);
	RefHandlesEmplace(CallId::ntdll_NtWriteVirtualMemory, hProcess);
	mwDetectProcessHollowing3(hProcess);
}

__middle_func__(MalwareId::ProcessHollowing, 4)
void on_NtSetContextThread(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hThread = (HANDLE)call->getArgument(0);
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


extern "C" __declspec(dllexport) 
void onApiCall(ApiCall* call)
{
	switch (call->getCallId())
	{
	case CallId::ntdll_NtCreateUserProcess:
		dbg("NtCreateUserProcess");
		on_NtCreateUserProcess(call);
		break;
	case CallId::ntdll_NtUnmapViewOfSection:
		dbg("NtUnmapViewOfSection");
		on_NtUnmapViewOfSection(call);
		break;
	case CallId::ntdll_NtWriteVirtualMemory:
		dbg("NtWriteVirtualMemory");
		on_NtWriteVirtualMemory(call);
		break;
	case CallId::ntdll_NtSetContextThread:
		dbg("NtSetContextThread");
		on_NtSetContextThread(call);
		break;
	case CallId::ntdll_NtResumeThread:
		dbg("NtResumeThread");
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
