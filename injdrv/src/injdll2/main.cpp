#define _CRT_SECURE_NO_WARNINGS

#include "ref_handles.h"
#include "handle_borne.h"
#include "mw_detect.h"
#include "string"


#define NT_SUCCESS(status) ((NTSTATUS)(status) >= 0)

#pragma warning( disable : 4311 )
#pragma warning( disable : 4302 )

bool IsWhiteListProcess(const wchar_t* image_path, size_t length);

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
__start_func__(MalwareId::SimpleProcessInjection)
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
	mwDetectProcessHollowing(hThread, call);
}

__trigger_func__(MalwareId::SimpleProcessInjection)
void on_NtCreateCreateThreadEx(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(3);
	mwDetectSimpleProcessInjection(hProcess, call);
}

__trigger_func__(MalwareId::SimpleProcessInjection)
void on_NtRtlCreateUserThread(ApiCall* call)
{
	if (call->isPost())
		return;

	HANDLE hProcess = (HANDLE)call->getArgument(0);
	mwDetectSimpleProcessInjection(hProcess, call);
}

// TODO change geistry path, windows 10
extern "C" __declspec(dllexport) 
void onApiCall(ApiCall* call, const wchar_t* image_path, size_t length)
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
	case CallId::ntdll_NtCreateThreadEx:
		on_NtCreateCreateThreadEx(call);
		break;
	case CallId::ntdll_RtlCreateUserThread:
		on_NtRtlCreateUserThread(call);
		break;
	default:
		break;
	}

	if (IsWhiteListProcess(image_path, length))
	{
		call->setMalwareId(MalwareId::None);
		call->skipCall(false);
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
	std::wstring path(image_path, length);

	for (auto& c : path)
		c = towlower(c);

	bool is_white = false;
	is_white = is_white || path.find(L"c:\\program files\\") == 0;
	is_white = is_white || path.find(L"c:\\program files (x86)\\") == 0;

	return is_white;
}
