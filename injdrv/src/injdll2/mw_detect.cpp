#include "mw_detect.h"
#include "ref_handles.h"
#include "handle_borne.h"

// CallId::ntdll_NtCreateUserProcess
// CallId::ntdll_NtUnmapViewOfSection
// CallId::ntdll_NtWriteProcessMemory
// CallId::ntdll_NtSetInformationThread
// CallId::ntdll_NtResumeThread
void mw_detect_process_hollowing(HANDLE h_thread, ApiCall* call)
{
	bool is_ref;

	is_ref = RefHandlesIsReferenced(CallId::ntdll_NtResumeThread, h_thread);
	if (!is_ref)
		return;

	is_ref = RefHandlesIsReferenced(CallId::ntdll_NtSetInformationThread, h_thread);
	if (!is_ref)
		return;

	HANDLE h_process = HandleBorneGetParent(h_thread);

	is_ref = RefHandlesIsReferenced(CallId::ntdll_NtWriteProcessMemory, h_process);
	if (!is_ref)
		return;

	is_ref = RefHandlesIsReferenced(CallId::ntdll_NtUnmapViewOfSection, h_process);
	if (!is_ref)
		return;

	is_ref = RefHandlesIsReferenced(CallId::ntdll_NtCreateUserProcess, h_process);
	if (!is_ref)
		return;

	call->setDangerousState();
}
