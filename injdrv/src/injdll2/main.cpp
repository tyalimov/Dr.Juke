#define _CRT_SECURE_NO_WARNINGS

#include "ref_handles.h"
#include "handle_borne.h"
#include "mw_detect.h"
#include <string>


#define NT_SUCCESS(status) ((NTSTATUS)(status) >= 0)

#pragma warning( disable : 4311 )
#pragma warning( disable : 4302 )

bool IsWhiteListProcess(const wchar_t* image_path, size_t length);

__parent_child__(hProcess, hThread)
__start_func__(MalwareId::ProcessHollowing)
__start_func__(MalwareId::EarlyBird)
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
		HandleBorneEraseReqursive(hProcess);
		RefHandlesErase(CallId::ntdll_NtUnmapViewOfSection, hProcess);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtUnmapViewOfSection, hProcess);

	bool detected = false;
	detected = mwDetectProcessHollowing2(hProcess);

	if (!detected)
	{
		HandleBorneEraseReqursive(hProcess);
		RefHandlesErase(CallId::ntdll_NtUnmapViewOfSection, hProcess);
		return;
	}
}

__middle_func__(MalwareId::ProcessHollowing, 3)
__start_func__(MalwareId::SimpleProcessInjection)
__start_func__(MalwareId::ApcInjection)
__middle_func__(MalwareId::EarlyBird, 2)
void on_NtWriteVirtualMemory(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(0);
	PVOID lpBaseAddress = (HANDLE)call->getArgument(1);
	if (!NT_SUCCESS(call->getReturnValue()))
	{
		HandleBorneEraseReqursive(hProcess);
		RefHandlesErase(CallId::ntdll_NtWriteVirtualMemory, hProcess);

		HandleBorneEraseReqursive(lpBaseAddress);
		RefHandlesErase(CallId::ntdll_NtWriteVirtualMemory, lpBaseAddress);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtWriteVirtualMemory, lpBaseAddress);
	RefHandlesEmplace(CallId::ntdll_NtWriteVirtualMemory, hProcess);

	bool detected = false;
	detected = detected || mwDetectProcessHollowing3(hProcess);
	detected = detected || mwDetectEarlyBird2(hProcess);

	if (!detected)
	{
		HandleBorneEraseReqursive(hProcess);
		RefHandlesErase(CallId::ntdll_NtWriteVirtualMemory, hProcess);
		return;
	}
}

__parent_child__(hThread, lpBaseAddress)
__middle_func__(MalwareId::ProcessHollowing, 4)
void on_NtSetContextThread(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hThread = (HANDLE)call->getArgument(0);

	if (!NT_SUCCESS(call->getReturnValue()))
	{
		HandleBorneEraseReqursive(hThread);
		RefHandlesErase(CallId::ntdll_NtSetContextThread, hThread);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtSetContextThread, hThread);
	
	bool detected = false;
	detected = detected || mwDetectProcessHollowing4(hThread);
	detected = detected || mwDetectThreadHijacking2(hThread);

	if (!detected)
	{
		HandleBorneEraseReqursive(hThread);
		RefHandlesErase(CallId::ntdll_NtSetContextThread, hThread);
		return;
	}
}

__trigger_func__(MalwareId::ProcessHollowing)
__trigger_func__(MalwareId::ThreadHijacking)
void on_NtResumeThread(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hThread = (HANDLE)call->getArgument(0);

	bool detected = false;
	detected = detected || mwDetectProcessHollowing(hThread, call);
	detected = detected || mwDetectThreadHijacking(hThread, call);

	if (!detected)
	{
		HandleBorneEraseReqursive(hThread);
		RefHandlesErase(CallId::ntdll_NtResumeThread, hThread);
		return;
	}
}

__trigger_func__(MalwareId::SimpleProcessInjection)
void on_NtCreateCreateThreadEx(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(3);

	bool detected = false;
	detected = mwDetectSimpleProcessInjection(hProcess, call);

	if (!detected)
	{
		HandleBorneEraseReqursive(hProcess);
		RefHandlesErase(CallId::ntdll_NtCreateThreadEx, hProcess);
		return;
	}
	
}

__trigger_func__(MalwareId::SimpleProcessInjection)
void on_RtlCreateUserThread(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(0);

	bool detected = false;
	detected = mwDetectSimpleProcessInjection(hProcess, call);

	if (!detected)
	{
		HandleBorneEraseReqursive(hProcess);
		RefHandlesErase(CallId::ntdll_RtlCreateUserThread, hProcess);
		return;
	}
}

__trigger_func__(MalwareId::ApcInjection)
__trigger_func__(MalwareId::EarlyBird)
void on_NtQueueApcThread(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hThread  = (HANDLE)call->getArgument(0);
	PVOID ApcRoutineContext  = (PVOID)call->getArgument(2);

	bool detected = false;
	detected = detected || mwDetectApcInjection(ApcRoutineContext, call);
	detected = detected || mwDetectEarlyBird(hThread, ApcRoutineContext, call);

	if (!detected)
	{
		HandleBorneEraseReqursive(hThread);
		RefHandlesErase(CallId::ntdll_NtQueueApcThread, hThread);

		HandleBorneEraseReqursive(ApcRoutineContext);
		RefHandlesErase(CallId::ntdll_NtQueueApcThread, ApcRoutineContext);
		return;
	}
}

__start_func__(MalwareId::ThreadHijacking)
void on_NtSuspendThread(ApiCall* call)
{
	if (call->isPre())
		return;

	HANDLE hThread = (HANDLE)call->getArgument(0);

	if (!NT_SUCCESS(call->getReturnValue()))
	{
		HandleBorneEraseReqursive(hThread);
		RefHandlesErase(CallId::ntdll_NtSuspendThread, hThread);
		return;
	}

	RefHandlesEmplace(CallId::ntdll_NtSuspendThread, hThread);
}

// TODO check on windows 10
// TODO cleanup after malware found

extern "C" __declspec(dllexport) 
void onApiCall(ApiCall* call, const wchar_t* image_path, size_t length)
{
	if (IsWhiteListProcess(image_path, length))
		return;

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
	case CallId::ntdll_NtCreateThreadEx:
		on_NtCreateCreateThreadEx(call);
		break;
	case CallId::ntdll_RtlCreateUserThread:
		on_RtlCreateUserThread(call);
		break;
	case CallId::ntdll_NtQueueApcThread:
		on_NtQueueApcThread(call);
		break;
	case CallId::ntdll_NtSuspendThread:
		on_NtSuspendThread(call);
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

bool IsWhiteListProcess(const wchar_t* image_path, size_t length)
{
	length /= sizeof(wchar_t);
	std::wstring path(image_path, length);

	for (auto& c : path)
		c = towlower(c);

	bool is_white = false;
	is_white = is_white || path.find(L"c:\\program files\\") == 0;
	is_white = is_white || path.find(L"c:\\program files (x86)\\") == 0;
	is_white = is_white || path.find(L"c:\\windows\\system32\\") == 0;
	is_white = is_white || path.find(L"c:\\windows\\syswow64\\") == 0;

	return is_white;
}
