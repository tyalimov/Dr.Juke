#include "mw_detect.h"
#include "ref_handles.h"
#include "handle_borne.h"

// CallId::ntdll_NtCreateUserProcess
// CallId::ntdll_NtUnmapViewOfSection
// CallId::ntdll_NtWriteProcessMemory
// CallId::ntdll_NtSetContextThread
// CallId::ntdll_NtResumeThread

#pragma warning( disable : 4312 )

#define STATUS_ACCESS_DENIED (ApiCall::arg_t)0xc0000022

#include <functional>
#include <vector>

using CheckFunc = bool(*)(HANDLE);

using namespace std;

// Process was create in suspended mode
bool mwDetectProcessHollowing2(HANDLE hProcess) {
	return RefHandlesIsReferenced(CallId::ntdll_NtCreateUserProcess, hProcess);
}

// Unmapped section of suspended process
bool mwDetectProcessHollowing3(HANDLE hProcess)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtUnmapViewOfSection, hProcess);
	return res && mwDetectProcessHollowing2(hProcess);
}

// shell code was written instead of mapped section
bool mwDetectProcessHollowing4(HANDLE hThread)
{
	HANDLE hProcess = HandleBorneGetParent(hThread);
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtWriteVirtualMemory, hProcess);

	return res && mwDetectProcessHollowing3(hProcess);
}

// was set rip to shell code buffer and now resuming thread
bool mwDetectProcessHollowing(HANDLE hThread, ApiCall* call)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtSetContextThread, hThread);
	res = res && mwDetectProcessHollowing4(hThread);

	if (res == true)
		call->setMalwareId(MalwareId::ProcessHollowing);

	return res;
}

// shell code written to process and thread created in this process
bool mwDetectSimpleProcessInjection(HANDLE hProcess, ApiCall* call)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtWriteVirtualMemory, hProcess);

	if (res == true)
		call->setMalwareId(MalwareId::SimpleProcessInjection);

	return res;
}


// on SetThreadContext -> is suspended?
bool mwDetectThreadHijacking2(HANDLE hThread) {
	return RefHandlesIsReferenced(CallId::ntdll_NtSuspendThread, hThread);
}

// on ResumeThread
bool mwDetectThreadHijacking(HANDLE hThread, ApiCall* call)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtSetContextThread, hThread);
	res = res && mwDetectThreadHijacking2(hThread);

	if (res == true)
		call->setMalwareId(MalwareId::ThreadHijacking);

	return res;
}

bool mwDetectApcInjection(HANDLE ApcRoutine, ApiCall* call)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtWriteVirtualMemory, ApcRoutine);

	if (res == true)
		call->setMalwareId(MalwareId::ApcInjection);

	return res;
}

bool mwDetectEarlyBird2(HANDLE hProcess) {
	return RefHandlesIsReferenced(CallId::ntdll_NtWriteVirtualMemory, hProcess);
}

bool mwDetectEarlyBird(HANDLE hThread, PVOID ApcRoutine, ApiCall* call)
{
	HANDLE hProcess = HandleBorneGetParent(hThread);
	bool res = mwDetectEarlyBird2(hProcess);
	res = res && RefHandlesIsReferenced(CallId::ntdll_NtWriteVirtualMemory, ApcRoutine);

	if (res == true)
		call->setMalwareId(MalwareId::EarlyBird);

	return res;
}
