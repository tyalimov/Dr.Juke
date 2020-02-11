#include "mw_detect.h"
#include "ref_handles.h"
#include "handle_borne.h"

// CallId::ntdll_NtCreateUserProcess
// CallId::ntdll_NtUnmapViewOfSection
// CallId::ntdll_NtWriteProcessMemory
// CallId::ntdll_NtSetContextThread
// CallId::ntdll_NtResumeThread

#include <functional>
#include <vector>

using CheckFunc = bool(*)(HANDLE);

using namespace std;

void mwCleanup(HANDLE handle)
{
	HandleBorneEraseReqursive(handle,
		[](HANDLE handle) {
			RefHandlesErase(CallId::ntdll_NtUnmapViewOfSection, handle);
		});
}

bool mwCleanupOnFalsePassOnTrue(HANDLE handle, bool res)
{
	if (res == false)
		mwCleanup(handle);

	return res;
}

bool mwDetectProcessHollowing2(HANDLE hProcess)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtCreateUserProcess, hProcess);
	return res;
}

bool mwDetectProcessHollowing3(HANDLE hProcess)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtUnmapViewOfSection, hProcess);

	if (mwCleanupOnFalsePassOnTrue(hProcess, res))
		res = mwDetectProcessHollowing2(hProcess);

	return res;
}

bool mwDetectProcessHollowing4(HANDLE hThread)
{
	HANDLE hProcess = HandleBorneGetParent(hThread);
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtWriteVirtualMemory, hProcess);

	if (mwCleanupOnFalsePassOnTrue(hThread, res))
		res = mwDetectProcessHollowing3(hProcess);

	return res;
}

bool mwDetectProcessHollowing(HANDLE hThread, ApiCall* call)
{
	bool res = RefHandlesIsReferenced(CallId::ntdll_NtSetContextThread, hThread);

	if (mwCleanupOnFalsePassOnTrue(hThread, res))
		res = mwDetectProcessHollowing4(hThread);

	if (res == true)
	{
		call->setMalwareId(MalwareId::ProcessHollowing);
		call->skipCall();
	}

	return res;
}
