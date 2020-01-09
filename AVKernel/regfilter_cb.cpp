#include "util.h"
#include "regfilter_cb.h"
#include "preferences.h"

#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/hash_set.h>

using namespace eastl;

using PID = HANDLE;

hash_map<wstring, ACCESS_MASK> gProtectedKeys;
hash_set<wstring> gAllowedProcImages;

GuardedMutex gRegFilterLock;

bool RegFilterAddProtectedKey(PWCH Name, 
	ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type)
{
	bool res = false;
	if (Type == REG_DWORD)
	{
		if (NameLen > 0 && DataLen > 0)
		{
			wstring KeyPath(Name, NameLen / sizeof(WCHAR));
			ACCESS_MASK Access = *(ACCESS_MASK*)Data;

			res = NormalizeRegistryPath(&KeyPath);
			if (res)
			{
				str_util::makeLower(&KeyPath);
				auto _pair = gProtectedKeys.try_emplace(KeyPath, Access);
				auto it = _pair.first;
				bool bInserted = _pair.second;

				if (bInserted)
				{
					kprintf(TRACE_INFO, "Added <Key=%ws>, <Access=0x%08X>",
						KeyPath.c_str(), Access);
				}
				else
				{
					it->second = Access;
					kprintf(TRACE_INFO, "Modified <Key=%ws>, <Access=0x%08X>",
						KeyPath.c_str(), Access);
				}

			}
			else
			{
				kprintf(TRACE_INFO, "Failed to add Key %ws",
					KeyPath.c_str());
			}
		}
	}

	return res;
}

void RegFilterRemoveProtectedKey(PWCH Name, ULONG NameLen)
{
	if (NameLen > 0)
	{
		wstring KeyPath(Name, NameLen / sizeof(WCHAR));
		bool res = NormalizeRegistryPath(&KeyPath);

		if (res)
		{
			str_util::makeLower(&KeyPath);
			auto n = gProtectedKeys.erase(KeyPath);

			if (n > 0)
				kprintf(TRACE_INFO, "Removed <Key=%ws>", KeyPath.c_str());
			else
				kprintf(TRACE_INFO, "Attempt to remove"
					"non existent <Key=%ws>", KeyPath.c_str());
		}
	}

}

bool RegFilterAddAllowedProcess(PWCH Name, ULONG NameLen)
{
	bool res = false;

	if (NameLen > 0)
	{
		wstring ImagePath(Name, NameLen / sizeof(WCHAR));
		gAllowedProcImages.emplace(ImagePath);

		// TODO: normalize path
		res = true;

		if (res)
			kprintf(TRACE_INFO, "Added Process <ImagePath=%ws>", ImagePath.c_str());
	}

	return res;
}

bool RegFilterRemoveAllowedProcess(PWCH Name, ULONG NameLen)
{
	bool res = false;

	if (NameLen > 0)
	{
		wstring ImagePath(Name, NameLen / sizeof(WCHAR));

		// TODO: normalize path
		res = true;

		auto n = gAllowedProcImages.erase(ImagePath);

		if (res)
		{
			if (n > 0)
				kprintf(TRACE_INFO, "Remove Process <ImagePath=%ws>", ImagePath.c_str());
			else
				kprintf(TRACE_INFO, "Attempt to remove"
					"non existent <ImagePath=%ws>", ImagePath.c_str());
		}
	}

	return res;
}

NTSTATUS OnRegFilterInit(PREGFILTER_CALLBACK_CTX CbContext)
{
	int i = 0;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(CbContext);

	Status = PreferencesReadFull(KRF_PROTECTED_KEYS,
		[&i](PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type) {

			bool res = RegFilterAddProtectedKey(Name, 
				NameLen, Data, DataLen, Type);

			if (res)
				i++;

		});

	if (i == 0)
		kprintf(TRACE_INFO, "No protected keys!");

	if (!NT_SUCCESS(Status))
		goto exit;
	
	i = 1;
	Status = PreferencesReadBasic(KRF_ALLOWED_PROCESSES,
		[&i](PWCH Name, ULONG NameLen) {

			bool res = RegFilterAddAllowedProcess(Name, NameLen);

			if (res)
				i++;

		});

	if (i - 1 == 0)
		kprintf(TRACE_INFO, "No allowed processes!");

exit:
	kprint_st(TRACE_REGFILTER_CB, Status);
	return Status;
}

NTSTATUS OnRegNtPostSetValueKey(
	PREG_POST_OPERATION_INFORMATION Info,
	PREGFILTER_CALLBACK_CTX CbContext,
	BOOLEAN bDeleted)
{
	NTSTATUS Status;
	PREG_SET_VALUE_KEY_INFORMATION PreInfo;
	PCUNICODE_STRING usObjectPath;

    PreInfo = (PREG_SET_VALUE_KEY_INFORMATION)Info->PreInformation;

	// Ignore failed operations
	if (!NT_SUCCESS(Info->Status))
		return STATUS_SUCCESS;

	// Get Absolute key name path
	Status = CmCallbackGetKeyObjectID(&CbContext->Cookie, 
		Info->Object, NULL, &usObjectPath);

	if (!NT_SUCCESS(Status))
	{
		kprintf(TRACE_REGFILTER, "CmCallbackGetKeyObjectID failed with status=0x%08X", Status);
		return STATUS_SUCCESS;
	}
	
	wstring KeyPath(usObjectPath->Buffer, usObjectPath->Length / sizeof(WCHAR));
	PUNICODE_STRING ValueName = PreInfo->ValueName;

	if (str_util::compareIns(KeyPath, KRF_PROTECTED_KEYS))
	{
		gRegFilterLock.acquire();

		if (bDeleted)
		{
			RegFilterRemoveProtectedKey(
				ValueName->Buffer, ValueName->Length);
		}
		else
		{
			RegFilterAddProtectedKey(
				ValueName->Buffer, ValueName->Length,
				PreInfo->Data, PreInfo->DataSize, PreInfo->Type);
		}

		gRegFilterLock.release();
	}
	else if (str_util::compareIns(KeyPath, KRF_ALLOWED_PROCESSES))
	{
		gRegFilterLock.acquire();
		
		if (bDeleted)
		{
			RegFilterRemoveAllowedProcess(
				ValueName->Buffer, ValueName->Length);
		}
		else
		{
			RegFilterAddAllowedProcess(
				ValueName->Buffer, ValueName->Length);
		}

		gRegFilterLock.release();
	}

	return STATUS_SUCCESS;
}

NTSTATUS OnRegNtPostDeleteValueKey(
	PREG_POST_OPERATION_INFORMATION Info,
	PREGFILTER_CALLBACK_CTX CbContext)
{
	NTSTATUS Status;
	PREG_SET_VALUE_KEY_INFORMATION PreInfo;
	PCUNICODE_STRING usObjectPath;

	PreInfo = (PREG_SET_VALUE_KEY_INFORMATION)Info->PreInformation;

	// Ignore failed operations
	if (!NT_SUCCESS(Info->Status))
		return STATUS_SUCCESS;

	// Get Absolute key name path
	Status = CmCallbackGetKeyObjectID(&CbContext->Cookie,
		Info->Object, NULL, &usObjectPath);

	if (!NT_SUCCESS(Status))
	{
		kprintf(TRACE_REGFILTER, "CmCallbackGetKeyObjectID failed with status=0x%08X", Status);
		return STATUS_SUCCESS;
	}

	wstring KeyPath(usObjectPath->Buffer, usObjectPath->Length / sizeof(WCHAR));
	

	return STATUS_SUCCESS;
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
	auto it = gProtectedKeys.find(KeyPath);

	while (pos != wstring::npos)
	{
		if (it != gProtectedKeys.end())
			return it;

		KeyPath = KeyPath.substr(0, pos);
		pos = KeyPath.rfind(L'\\');
		it = gProtectedKeys.find(KeyPath);
	}

	return gProtectedKeys.end();
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
			KeyPath.insert(0, move(ObjectPath));
		}
	}

	// Lowercase registry key path
	// And find rule matchin the key
	str_util::makeLower(&KeyPath);	
	auto it = RegFilterFindRule(KeyPath);

	if (it != gProtectedKeys.end())
	{
		ACCESS_MASK AccessMask = it->second;
		return RegFilterAccessCheck(KeyPath,
			AccessMask, Info->DesiredAccess);
	}	

	return STATUS_SUCCESS;
}
