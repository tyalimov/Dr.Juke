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
				CHAR* kvfi_ch = new CHAR[DataSize];
				KEY_VALUE_FULL_INFORMATION* kvfi = (KEY_VALUE_FULL_INFORMATION*)kvfi_ch;
				if (kvfi)
				{
					Status = ZwEnumerateValueKey(KeyHandle, idx, KeyValueFullInformation, kvfi, DataSize, &DataSize);
					if (NT_SUCCESS(Status))
					{
						onRecord(
							kvfi->Name, kvfi->NameLength,
							(PVOID)(kvfi_ch + kvfi->DataOffset), 
							kvfi->DataLength, kvfi->Type
						);
					}
					else
						kprintf(TRACE_ERROR, "ZwEnumerateValueKey failed");

					delete[] kvfi_ch;
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
