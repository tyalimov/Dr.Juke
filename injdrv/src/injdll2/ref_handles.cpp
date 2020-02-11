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
};


void RefHandlesEmplace(CallId id, HANDLE handle)
{
	lock_guard<mutex> guard(g_rh_lock);

	int i = static_cast<int>(id);
	auto& handles = g_ref_handles[i].handles;
	handles.emplace(handle);
}

bool RefHandlesIsReferenced(CallId id, HANDLE handle)
{
	lock_guard<mutex> guard(g_rh_lock);

	int i = static_cast<int>(id);
	auto& handles = g_ref_handles[i].handles;

	auto it = handles.find(handle);
	return it != handles.end();
}

void RefHandlesErase(CallId id, HANDLE handle)
{
	lock_guard<mutex> guard(g_rh_lock);

	int i = static_cast<int>(id);
	auto& handles = g_ref_handles[i].handles;
	handles.erase(handle);
}
