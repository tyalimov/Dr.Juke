#include "preferences.h"

using namespace eastl;

NTSTATUS PreferencesReadFull(LPCWSTR AbsRegPath, 
	function<void(PWCH Name, ULONG NameLength, PVOID Data, ULONG DataLength, ULONG Type)> onRecord)
{
	NTSTATUS Status;
	HANDLE KeyHandle;
	OBJECT_ATTRIBUTES Attrs;
	UNICODE_STRING KeyPath;
	ULONG DataSize;
	ULONG idx = 0;

	RtlInitUnicodeString(&KeyPath, AbsRegPath);
	InitializeObjectAttributes(&Attrs, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwOpenKeyEx(&KeyHandle, KEY_READ, &Attrs, 0);

	if (NT_SUCCESS(Status))
	{
		while (1)
		{
			DataSize = 0;
			Status = ZwEnumerateValueKey(KeyHandle, idx, KeyValueFullInformation, NULL, 0, &DataSize);
			if (Status == STATUS_NO_MORE_ENTRIES)
			{
				Status = STATUS_SUCCESS;
				break;
			}

			if (Status != STATUS_BUFFER_TOO_SMALL && Status != STATUS_BUFFER_OVERFLOW)
				break;

			if (DataSize > 0)
			{
				CHAR* InfoBuffer = new CHAR[DataSize];
				KEY_VALUE_FULL_INFORMATION* Info = (KEY_VALUE_FULL_INFORMATION*)InfoBuffer;
				if (Info)
				{
					Status = ZwEnumerateValueKey(KeyHandle, idx, KeyValueFullInformation, Info, DataSize, &DataSize);
					if (NT_SUCCESS(Status))
					{
						onRecord(
							Info->Name, Info->NameLength,
							(PVOID)(InfoBuffer + Info->DataOffset), 
							Info->DataLength, Info->Type
						);
					}
					else
						kprintf(TRACE_ERROR, "ZwEnumerateValueKey failed");

					delete[] InfoBuffer;
				}
				else
					kprintf(TRACE_ERROR, "Operator new returned nullptr");
			}
			else
				kprintf(TRACE_ERROR, "Empty record");
				
			idx++;
		}

		ZwClose(KeyHandle);
	}
	else
		kprintf(TRACE_ERROR, "Failed to open key %ws", AbsRegPath);


	kprintf(TRACE_PREF, "Read %d entries. Exit with Status 0x%08X", idx, Status);
	return Status;
}

NTSTATUS PreferencesReadBasic(LPCWSTR AbsRegPath, 
	function<void(PWCH Name, ULONG NameLength)> onRecord)
{
	NTSTATUS Status;
	HANDLE KeyHandle;
	OBJECT_ATTRIBUTES Attrs;
	UNICODE_STRING KeyPath;
	ULONG DataSize;
	ULONG idx = 0;

	RtlInitUnicodeString(&KeyPath, AbsRegPath);
	InitializeObjectAttributes(&Attrs, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwOpenKeyEx(&KeyHandle, KEY_READ, &Attrs, 0);

	if (NT_SUCCESS(Status))
	{
		while (1)
		{
			DataSize = 0;
			Status = ZwEnumerateValueKey(KeyHandle, idx, KeyValueBasicInformation, NULL, 0, &DataSize);
			if (Status == STATUS_NO_MORE_ENTRIES)
			{
				Status = STATUS_SUCCESS;
				break;
			}

			if (Status != STATUS_BUFFER_TOO_SMALL && Status != STATUS_BUFFER_OVERFLOW)
				break;

			if (DataSize > 0)
			{
				CHAR* InfoBuffer = new CHAR[DataSize];
				KEY_VALUE_BASIC_INFORMATION* Info = (KEY_VALUE_BASIC_INFORMATION*)InfoBuffer;
				if (Info)
				{
					Status = ZwEnumerateValueKey(KeyHandle, idx, KeyValueBasicInformation, Info, DataSize, &DataSize);
					if (NT_SUCCESS(Status))
						onRecord(Info->Name, Info->NameLength);
					else
						kprintf(TRACE_ERROR, "ZwEnumerateValueKey failed");

					delete[] InfoBuffer;
				}
				else
					kprintf(TRACE_ERROR, "Operator new returned nullptr");
			}
			else
				kprintf(TRACE_ERROR, "Empty record");
				
			idx++;
		}

		ZwClose(KeyHandle);
	}
	else
		kprintf(TRACE_ERROR, "Failed to open key %ws", AbsRegPath);


	kprintf(TRACE_PREF, "Read %d entries. Exit with Status 0x%08X", idx, Status);
	return Status;
}

NTSTATUS PreferencesQueryKeyValue(const wchar_t* szAbsRegPath, 
	const wchar_t* szValueName, function<NTSTATUS(PKEY_VALUE_FULL_INFORMATION)> onRecord)
{
	NTSTATUS Status;
	HANDLE KeyHandle;
	OBJECT_ATTRIBUTES Attrs;
	UNICODE_STRING KeyPath;
	UNICODE_STRING ValueName;
	ULONG Size = 0;
	CHAR* InfoBuffer;
	PKEY_VALUE_FULL_INFORMATION Info;

	RtlInitUnicodeString(&KeyPath, szAbsRegPath);
	InitializeObjectAttributes(&Attrs, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwOpenKeyEx(&KeyHandle, KEY_READ, &Attrs, 0);
	if (!NT_SUCCESS(Status))
	{
		kprintf(TRACE_ERROR, 
			"ZwOpenKeyEx failed <Status=0x%08X>", Status);

		return Status;
	}

	RtlInitUnicodeString(&ValueName, szValueName);
	Status = ZwQueryValueKey(KeyHandle, &ValueName,
		KeyValueFullInformation, NULL, Size, &Size);

	if (Status != STATUS_BUFFER_TOO_SMALL && Status != STATUS_BUFFER_OVERFLOW)
	{
		kprintf(TRACE_ERROR, 
			"ZwQueryValueKey failed <Status=0x%08X>", Status);

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

	kprint_st(TRACE_PREF, Status);
	return Status;
}
