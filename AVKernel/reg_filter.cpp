#include "reg_filter.h"
#include "preferences.h"
#include "util.h"
#include <EASTL/string.h>
#include <EASTL/hash_set.h>
#include <EASTL/hash_map.h>

//------------------------------------------------------------>
// Prototypes

static NTSTATUS 
RegFilterCallback(
	_In_     PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2);

static NTSTATUS 
RegPostSetValueKey(
	_In_ PREGFILTER_CALLBACK_CTX CbContext,
	_Inout_	PVOID Argument2,
	_In_ BOOLEAN bDeleted);

static NTSTATUS 
RegPreCreateKeyEx(
	_In_ PREGFILTER_CALLBACK_CTX CallbackCtx,
	_Inout_ PVOID Argument2);

static NTSTATUS
RegFilterAccessCheck(const wstring& KeyPath,
	DWORD32 ActualAccess, DWORD32 DesiredAccess);

static auto
RegFilterFindRule(wstring KeyPath);

//------------------------------------------------------------>
// Registry filter

hash_map<wstring, ACCESS_MASK> gProtectedKeys;
hash_set<wstring> gAllowedProcImages;

GuardedMutex gRegFilterLock;

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject, PREGFILTER_CALLBACK_CTX CbContext)
{
	int i = 0;
	NTSTATUS Status;
	ULONG MajorVersion = 0, MinorVersion = 0;
	UNICODE_STRING Altitude = RTL_CONSTANT_STRING(REGFILTER_ALTITUDE);

    CmGetCallbackVersion(&MajorVersion, &MinorVersion);
    kprintf(TRACE_REGFILTER, "Callback version %u.%u", MajorVersion, MinorVersion);


	Status = CmRegisterCallbackEx(
		RegFilterCallback,
		&Altitude,
		DriverObject,
		(PVOID)CbContext,
		&CbContext->Cookie,
		NULL);

	if (NT_SUCCESS(Status))
	{
		CbContext->IsInitialized = TRUE;
	}

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
	kprint_st(TRACE_REGFILTER, Status);
	return Status;
}

void RegFilterExit(PREGFILTER_CALLBACK_CTX CbContext)
{
	NTSTATUS Status = CmUnRegisterCallback(CbContext->Cookie);
	CbContext->IsInitialized = FALSE;
	kprint_st(TRACE_REGFILTER, Status);
}

static NTSTATUS
RegFilterCallback(
	_In_  PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	REG_NOTIFY_CLASS NotifyClass;
	PREGFILTER_CALLBACK_CTX CbContext;

	CbContext = (PREGFILTER_CALLBACK_CTX)CallbackContext;
	NotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;

	// Ignore windows system process
	if (PsGetCurrentProcessId() == (HANDLE)4)
		return STATUS_SUCCESS;

	if (Argument2 == NULL)
	{
		kprintf(TRACE_REGFILTER, "Argument 2 unexpectedly 0. Abort with STATUS_SUCCESS");
		return STATUS_SUCCESS;
	}

	switch (NotifyClass)
	{
	case RegNtPreCreateKeyEx: 
	case RegNtPreOpenKeyEx: 
		if (false) Status = RegPreCreateKeyEx(CbContext, Argument2);
		break;
	case RegNtPostSetValueKey: 
		if (false) Status = RegPostSetValueKey(CbContext, Argument2, FALSE);
		break;
	case RegNtPostDeleteValueKey:
		if (false) Status = RegPostSetValueKey(CbContext, Argument2, TRUE);
		break;
	default: 
		break;
	}

	return Status;
}

static auto 
RegFilterFindRule(wstring KeyPath)
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

static NTSTATUS RegFilterAccessCheck(const wstring& KeyPath, 
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

static NTSTATUS 
RegPreCreateKeyEx(
    _In_ PREGFILTER_CALLBACK_CTX CbContext,
    _Inout_	PVOID Argument2)
{
	NTSTATUS Status;
    PREG_CREATE_KEY_INFORMATION_V1 PreCreateInfo;
	  
	PreCreateInfo = (PREG_CREATE_KEY_INFORMATION_V1) Argument2;
	if ((ULONG_PTR)PreCreateInfo->Version != 1)
	{
		kprintf(TRACE_REGFILTER, "PreCreateInfo type is not PREG_CREATE_KEY_INFORMATION_V1");
		return STATUS_SUCCESS;
	}

	PUNICODE_STRING usKeyPath = PreCreateInfo->CompleteName;
	
	wstring KeyPath(usKeyPath->Buffer, usKeyPath->Length / sizeof(WCHAR));
	if (KeyPath[0] != L'\\')
	{
		// Convert relative path to absolute
		PCUNICODE_STRING usObjectPath;
		Status = CmCallbackGetKeyObjectID(&CbContext->Cookie,
			PreCreateInfo->RootObject, NULL, &usObjectPath);

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
	// And find rule matching the key
	str_util::makeLower(&KeyPath);	
	auto it = RegFilterFindRule(KeyPath);

	if (it != gProtectedKeys.end())
	{
		ACCESS_MASK AccessMask = it->second;
		return RegFilterAccessCheck(KeyPath,
			AccessMask, PreCreateInfo->DesiredAccess);
	}	

	return STATUS_SUCCESS;
}

static NTSTATUS 
RegPostSetValueKey(
	_In_ PREGFILTER_CALLBACK_CTX CbContext,
	_Inout_	PVOID Argument2,
	_In_ BOOLEAN bDeleted)
{
	NTSTATUS Status;
	PCUNICODE_STRING KeyPath;
	PREG_SET_VALUE_KEY_INFORMATION PreInfo;
	PREG_POST_OPERATION_INFORMATION PostInfo;
	PostInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
    PreInfo = (PREG_SET_VALUE_KEY_INFORMATION)PostInfo->PreInformation;

	// Ignore failed operations
	if (!NT_SUCCESS(PostInfo->Status))
		return STATUS_SUCCESS;

	// Get Absolute key name path
	Status = CmCallbackGetKeyObjectID(&CbContext->Cookie, 
		PostInfo->Object, NULL, &KeyPath);

	if (!NT_SUCCESS(Status))
	{
		kprintf(TRACE_REGFILTER, "CmCallbackGetKeyObjectID failed with status=0x%08X", Status);
		return STATUS_SUCCESS;
	}
	
	UNICODE_STRING ProtectedKeys = RTL_CONSTANT_STRING(KRF_PROTECTED_KEYS);
	UNICODE_STRING AllowedProcesses = RTL_CONSTANT_STRING(KRF_ALLOWED_PROCESSES);
	PUNICODE_STRING ValueName = PreInfo->ValueName;

	if (RtlCompareUnicodeString(KeyPath, &ProtectedKeys, TRUE) == 0)
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
	else if (RtlCompareUnicodeString(KeyPath, &AllowedProcesses, TRUE) == 0)
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

	if (CbContext->onKeyChange)
		CbContext->onKeyChange(PostInfo, CbContext, bDeleted);

	return STATUS_SUCCESS;
}

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
