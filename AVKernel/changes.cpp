#pragma once

#include "changes.h"
// #include "filter.h"

#include <EASTL/string.h>

using namespace eastl;

void OnKeyAppsChange()
{
	int i = 0;

	PreferencesReadFull(KEY_APPS,
		[&i](PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type) {

			if (Type == REG_SZ)
			{
				if (NameLen > 0)
				{
					wstring ws(Name, NameLen);
					kprintf(TRACE_CHANGES, "%d) Name=%ws", i, ws.c_str());
				}
				
				if (DataLen > 0)
				{
					wstring ws((PWCH)Data, DataLen);
					kprintf(TRACE_CHANGES, "%d) Data=%ws", i, ws.c_str());
				}
			}

			i++;

		});	
}

void OnKeyTotalcmdChange()
{
	int i = 0;

	PreferencesReadFull(KEY_TOTALCMD,
		[&i](PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type) {

			if (Type == REG_SZ)
			{
				if (NameLen > 0)
				{
					wstring ws(Name, NameLen);
					kprintf(TRACE_CHANGES, "%d) Name=%ws", i, ws.c_str());
				}
				
				if (DataLen > 0)
				{
					wstring ws((PWCH)Data, DataLen);
					kprintf(TRACE_CHANGES, "%d) Data=%ws", i, ws.c_str());
				}
			}

			i++;

		});	
}
