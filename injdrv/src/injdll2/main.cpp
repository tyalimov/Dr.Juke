#define _CRT_SECURE_NO_WARNINGS

#include "ref_handles.h"
#include "handle_borne.h"
#include <string>

#define MAX_SIZE 50000
#define __start_func__
#define __trigger_func__

using namespace std;

__start_func__ void
on_NtCreateUserProcess(ApiCall* call)
{
	if (!call->isPost())
		return;

	PHANDLE ph_process;
	PHANDLE ph_thread;
	HANDLE h_process;
	HANDLE h_thread;


	ph_process = (PHANDLE)call->getArgument(0);
	ph_thread = (PHANDLE)call->getArgument(1);
	h_process = ph_process ? *ph_process : NULL;
	h_thread = ph_thread ? *ph_thread : NULL;

	if (h_process && h_thread)
		HandleBorneEmplace(h_process, h_thread);

	if (h_process != NULL)
		RefHandlesEmplace(CallId::ntdll_NtCreateUserProcess, h_process);
}

void on_NtUnmapViewOfSection(ApiCall* call)
{
	if (!call->isPost())
		return;

	PHANDLE ph_process;
	HANDLE h_process;

	ph_process = (PHANDLE)call->getArgument(0);
	h_process = ph_process ? *ph_process : NULL;

	if (h_process != NULL)
		RefHandlesEmplace(CallId::ntdll_NtUnmapViewOfSection, h_process);
}

void on_NtWriteProcessMemory(ApiCall* call)
{
	if (!call->isPost())
		return;

	PHANDLE ph_process;
	HANDLE h_process;

	ph_process = (PHANDLE)call->getArgument(0);
	h_process = ph_process ? *ph_process : NULL;

	if (h_process != NULL)
		RefHandlesEmplace(CallId::ntdll_NtWriteProcessMemory, h_process);
}

void on_NtSetInformationThread(ApiCall* call)
{
	if (!call->isPost())
		return;

	PHANDLE ph_thread;
	HANDLE h_thread;

	ph_thread = (PHANDLE)call->getArgument(0);
	h_thread = ph_thread ? *ph_thread : NULL;

	if (h_thread != NULL)
		RefHandlesEmplace(CallId::ntdll_NtSetInformationThread, h_thread);
}

__trigger_func__ void
on_NtResumeThread(ApiCall* call)
{
	if (!call->isPost())
		return;

	PHANDLE ph_thread;
	HANDLE h_thread;

	ph_thread = (PHANDLE)call->getArgument(0);
	h_thread = ph_thread ? *ph_thread : NULL;

	if (h_thread != NULL)
		RefHandlesEmplace(CallId::ntdll_NtResumeThread, h_thread);
}


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
	case CallId::ntdll_NtWriteProcessMemory:
		on_NtWriteProcessMemory(call);
		break;
	case CallId::ntdll_NtSetInformationThread:
		on_NtSetInformationThread(call);
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
