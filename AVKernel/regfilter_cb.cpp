#include "util.h"
#include "regfilter_cb.h"
#include "preferences.h"

#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/set.h>

using namespace eastl;

using PID = HANDLE;

hash_map<wstring, ACCESS_MASK> UserKeys;
set<wstring> SystemKeys;
set<PID> AllowedProcesses;

NTSTATUS OnRegFilterInit(PREGFILTER_CALLBACK_CTX CbContext)
{
	int i = 1;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(CbContext);

	Status = PreferencesReadFull(KEY_TOTALCMD,
		[&i](PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type) {

			if (Type == REG_DWORD)
			{
				if (NameLen > 0 && DataLen > 0)
				{
					wstring KeyPath(Name, NameLen / sizeof(WCHAR));
					ACCESS_MASK Access = *(ACCESS_MASK*)Data;

					bool res = NormalizeRegistryPath(&KeyPath);
					if (res)
					{
						str_util::makeLower(&KeyPath);
						UserKeys.emplace(KeyPath, Access);
						kprintf(TRACE_INFO, "%d) KeyPath=%ws Access=0x%08X",
							i, KeyPath.c_str(), Access);
					}
				}

				i++;
			}
		});

	if (i - 1 == 0)
		kprintf(TRACE_INFO, "Empty list");

	kprint_st(TRACE_REGFILTER_CB, Status);
	return Status;
}

NTSTATUS OnRegNtPostSetValueKey(
	PREG_POST_OPERATION_INFORMATION Info,
	PREGFILTER_CALLBACK_CTX CbContext)
{
	UNREFERENCED_PARAMETER(Info);
	UNREFERENCED_PARAMETER(CbContext);
	NTSTATUS Status = STATUS_SUCCESS;
//	
//	Status = ObOpenObjectByPointer(info->Object,
//								   OBJ_KERNEL_HANDLE,
//								   NULL,
//								   KEY_ALL_ACCESS,
//								   PreCreateInfo->ObjectType,
//								   KernelMode,
//								   &Key);
//
//	if (!NT_SUCCESS (Status)) {
//		ErrorPrint("ObObjectByPointer failed. Status 0x%x\n", Status);
//		break;
//	}
//
//	Status = ZwDeleteKey(Key);
//
//	if (!NT_SUCCESS(Status)) {
//		ErrorPrint("ZwDeleteKey failed. Status 0x%x\n", Status);
//		break;
//	}
//
//	ZwClose(Key);

	return Status;
}

NTSTATUS RegFilterAccessCheck(const wstring& KeyPath, 
	DWORD32 ActualAccess, DWORD32 DesiredAccess)
{
	if (DesiredAccess & ActualAccess)
	{
		kprintf(TRACE_REGFILTER, "Allow <pid=%d> to access %ws", 
			PsGetCurrentProcessId(), KeyPath.c_str());

		return STATUS_SUCCESS;
	}
	else
	{
		kprintf(TRACE_REGFILTER, "Deny <pid=%d> to access %ws", 
			PsGetCurrentProcessId(), KeyPath.c_str());

		return STATUS_ACCESS_DENIED;
	}
}

auto RegFilterFindRule(wstring KeyPath)
{
	auto pos = KeyPath.rfind(L'\\');
	auto it = UserKeys.find(KeyPath);

	while (pos != wstring::npos)
	{
		if (it != UserKeys.end())
			return it;

		KeyPath = KeyPath.substr(0, pos);
		pos = KeyPath.rfind(L'\\');
		it = UserKeys.find(KeyPath);
	}

	return UserKeys.end();
}

NTSTATUS OnRegNtPreCreateKeyEx(
	PREG_CREATE_KEY_INFORMATION_V1 Info,
	PREGFILTER_CALLBACK_CTX CbContext)
{
	NTSTATUS Status;
	PUNICODE_STRING usKeyPath = Info->CompleteName;
	
	wstring KeyPath(usKeyPath->Buffer, usKeyPath->Length / sizeof(WCHAR));
	if (KeyPath[0] != L'\\')
	{
		// Convert relative path to absolute
		PCUNICODE_STRING usObjectPath;
		Status = CmCallbackGetKeyObjectID(&CbContext->Cookie, Info->RootObject, NULL, &usObjectPath);

		if (!NT_SUCCESS(Status))
		{
			kprintf(TRACE_REGFILTER, "CmCallbackGetKeyObjectID failed with status=0x%08X", Status);
			return STATUS_SUCCESS;
		}
		else
		{
			wstring ObjectPath(usObjectPath->Buffer, usObjectPath->Length / sizeof(WCHAR));
			ObjectPath.append(L"\\");
			KeyPath.insert(0, ObjectPath);
		}
	}

	// Lowercase registry key path
	// And find rule matchin the key
	str_util::makeLower(&KeyPath);	
	auto it = RegFilterFindRule(KeyPath);

	if (it != UserKeys.end())
	{
		ACCESS_MASK AccessMask = it->second;
		return RegFilterAccessCheck(KeyPath,
			AccessMask, Info->DesiredAccess);
	}	

	return STATUS_SUCCESS;
}

// set<ULONG> gSecuredProcesses;
// GuardedMutex mutex;
// 
// void OnProcFilterKeyChange()
// {
// 	LockGuard<GuardedMutex> lock(&mutex);
// 	gSecuredProcesses.clear();
// 
// 	int i = 0;
// 	PreferencesReadBasic(KEY_PROCFILTER,
// 		[&i](PWCH Name, ULONG NameLength) {
// 			
// 			if (NameLength > 0)
// 			{
// 				wstring ws(Name, NameLength);
// 				ULONG pid = _wtoi(ws.c_str());
// 				gSecuredProcesses.emplace(pid);
// 				i++;
// 			}
// 
// 		});
// 
// 	kprintf(TRACE_NOTIFIER, "Updated %d entries", i);
// }



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

