#include "util.h"
#include "trace.h"
#include "wow64log.h"

VOID NTAPI
ProtectDll(HANDLE ModuleHandle)
{

	//
	// Make us unloadable (by FreeLibrary calls).
	//

	LdrAddRefDll(LDR_ADDREF_DLL_PIN, ModuleHandle);

	//
	// Hide this DLL from the PEB.
	//

	PPEB Peb = NtCurrentPeb();
	PLIST_ENTRY ListEntry;

	for (ListEntry = Peb->Ldr->InLoadOrderModuleList.Flink;
		ListEntry != &Peb->Ldr->InLoadOrderModuleList;
		ListEntry = ListEntry->Flink)
	{
		PLDR_DATA_TABLE_ENTRY LdrEntry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		//
		// ModuleHandle is same as DLL base address.
		//

		if (LdrEntry->DllBase == ModuleHandle)
		{
			RemoveEntryList(&LdrEntry->InLoadOrderLinks);
			RemoveEntryList(&LdrEntry->InInitializationOrderLinks);
			RemoveEntryList(&LdrEntry->InMemoryOrderLinks);
			RemoveEntryList(&LdrEntry->HashLinks);

			break;
		}
	}

	//
	// Create exports for Wow64Log* functions in
	// the PE header of this DLL.
	//

	Wow64LogCreateExports(ModuleHandle);
}

NTSTATUS NTAPI
QueryKeyValue(const wchar_t* szAbsRegPath, 
	const wchar_t* szValueName, NTSTATUS(*onRecord)(PKEY_VALUE_FULL_INFORMATION))
{
	NTSTATUS Status;
	HANDLE KeyHandle;
	OBJECT_ATTRIBUTES Attrs;
	UNICODE_STRING KeyPath;
	UNICODE_STRING ValueName;
	ULONG Size = 0;
	CHAR* InfoBuffer;
	PKEY_VALUE_FULL_INFORMATION Info;

	RtlInitUnicodeString(&KeyPath, (PWCH)szAbsRegPath);
	InitializeObjectAttributes(&Attrs, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = NtOpenKeyEx(&KeyHandle, KEY_READ, &Attrs, 0);
	if (!NT_SUCCESS(Status))
	{
		Trace::logError(L"NtOpenKeyEx failed <Status=0x%08X>", Status);
		return Status;
	}

	RtlInitUnicodeString(&ValueName, (PWSTR)szValueName);
	Status = ZwQueryValueKey(KeyHandle, &ValueName,
		KeyValueFullInformation, NULL, Size, &Size);

	if (Status != STATUS_BUFFER_TOO_SMALL && Status != STATUS_BUFFER_OVERFLOW)
	{
		Trace::logError(L"ZwQueryValueKey failed <Status=0x%08X>", Status);
		return Status;
	}

	if (Size > 0)
	{
		InfoBuffer = new CHAR[Size];
		if (InfoBuffer)
		{
			Info = (PKEY_VALUE_FULL_INFORMATION)InfoBuffer;
			Status = ZwQueryValueKey(KeyHandle, &ValueName, 
				KeyValueFullInformation, InfoBuffer, Size, &Size);

			if (NT_SUCCESS(Status))
				Status = onRecord(Info);

			delete[] InfoBuffer;
		}
		else
			Status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
		Status = STATUS_BUFFER_TOO_SMALL;

	ZwClose(KeyHandle);
	return Status;
}
