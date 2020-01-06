
#include "filter.h"
#include "util.h"
#include "preferences.h"

#include <EASTL/unique_ptr.h>
#include <EASTL/string.h>
#include <EASTL/set.h>

using namespace eastl;

set<ULONG> gSecuredProcesses;
GuardedMutex mutex;

void OnProcFilterKeyChange()
{
	LockGuard<GuardedMutex> lock(&mutex);
	gSecuredProcesses.clear();

	int i = 0;
	PreferencesReadBasic(KEY_PROCFILTER,
		[&i](PWCH Name, ULONG NameLength) {
			
			if (NameLength > 0)
			{
				wstring ws(Name, NameLength);
				ULONG pid = _wtoi(ws.c_str());
				gSecuredProcesses.emplace(pid);
				i++;
			}

		});

	kprintf(TRACE_NOTIFIER, "Updated %d entries", i);
}



// void OnKeyTotalcmdChange()
// {
// 	int i = 0;
// 
// 	PreferencesReadFull(KEY_TOTALCMD,
// 		[&i](PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type) {
// 
// 			if (Type == REG_SZ)
// 			{
// 				if (NameLen > 0)
// 				{
// 					wstring ws(Name, NameLen);
// 					kprintf(TRACE_CHANGES, "%d) Name=%ws", i, ws.c_str());
// 				}
// 				
// 				if (DataLen > 0)
// 				{
// 					wstring ws((PWCH)Data, DataLen);
// 					kprintf(TRACE_CHANGES, "%d) Data=%ws", i, ws.c_str());
// 				}
// 			}
// 
// 			i++;
// 
// 		});	
// }
// 
