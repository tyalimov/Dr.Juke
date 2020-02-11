#include "ref_handles.h"
#include <mutex>
#include <set>

using namespace std;

struct ReferencedHandles
{
	CallId id;
	set<HANDLE> handles;
};

mutex g_rh_lock;
ReferencedHandles g_ref_handles[] = {
	{ CallId::ntdll_NtCreateUserProcess, set<HANDLE>() },
	{ CallId::ntdll_NtWriteVirtualMemory, set<HANDLE>() },
	{ CallId::ntdll_NtUnmapViewOfSection, set<HANDLE>() },
	{ CallId::ntdll_NtSetContextThread, set<HANDLE>() },
	{ CallId::ntdll_NtResumeThread, set<HANDLE>() },
	{ CallId::ntdll_NtCreateThreadEx, set<HANDLE>() },
	{ CallId::ntdll_RtlCreateUserThread, set<HANDLE>() },
};

CallId idx_start = CallId::ntdll_NtCreateUserProcess;
CallId idx_end = CallId::ntdll_RtlCreateUserThread;

void RefHandlesEmplace(CallId id, HANDLE handle)
{
	lock_guard<mutex> guard(g_rh_lock);

	if (id < idx_start || id > idx_end)
		return;

	int i = static_cast<int>(id);
	auto& handles = g_ref_handles[i].handles;
	handles.emplace(handle);
}

bool RefHandlesIsReferenced(CallId id, HANDLE handle)
{
	lock_guard<mutex> guard(g_rh_lock);

	if (id < idx_start || id > idx_end)
		return false;

	int i = static_cast<int>(id);
	auto& handles = g_ref_handles[i].handles;

	auto it = handles.find(handle);
	return it != handles.end();
}

void RefHandlesErase(CallId id, HANDLE handle)
{
	lock_guard<mutex> guard(g_rh_lock);

	if (id < idx_start || id > idx_end)
		return;

	int i = static_cast<int>(id);
	auto& handles = g_ref_handles[i].handles;
	handles.erase(handle);
}
