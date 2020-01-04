#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>

#include "common.h"
#include "util.h"

#include <EASTL/unique_ptr.h>

#include <EASTL/string.h>

using namespace eastl;

// class RegistryReader
// {
// 
// private:
// 
// 	wstring m_path;
// 
// public:
// 
// 	RegistryKey(LPCWSTR path) : m_path(path) {}
// 	~RegistryKey() = default;
// 
// 
// 	void create();
// 	void remove();
// 	void enumerateKeys();
// 	void enumerateKeyValues();
// 	void getValue();
// 
// 	NTSTATUS setValue(LPCWSTR val_name, ULONG val_type, PVOID data, ULONG data_len)
// 	{
// 		return RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE, 
// 			m_path.c_str(), val_name, val_type, data, data_len);
// 	}
// 
// 	void setChangeNotifier();
// };

// NTSYSAPI NTSTATUS ZwNotifyChangeKey(
//   HANDLE           KeyHandle,
//   HANDLE           Event,
//   PIO_APC_ROUTINE  ApcRoutine,
//   PVOID            ApcContext,
//   PIO_STATUS_BLOCK IoStatusBlock,
//   ULONG            CompletionFilter,
//   BOOLEAN          WatchTree,
//   PVOID            Buffer,
//   ULONG            BufferSize,
//   BOOLEAN          Asynchronous
// );

BOOLEAN ReadPreferences(LPCWSTR reg_path)
{
	BOOLEAN ok;
	NTSTATUS status;
	HANDLE hKey;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING KeyPath;

	UNREFERENCED_PARAMETER(reg_path);

	ok = FALSE;
	//HKEY_LOCAL_MACHINE\SOFTWARE\RegisteredApplications
	RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\SOFTWARE\\RegisteredApplications");
	InitializeObjectAttributes(&oa, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKeyEx(&hKey, KEY_READ, &oa, 0);

	if (NT_SUCCESS(status))
	{
		ULONG data_size;
		ULONG idx = 0;

		while (1)
		{
			data_size = 0;
			status = ZwEnumerateValueKey(hKey, idx, KeyValueFullInformation, NULL, 0, &data_size);
			if (status == STATUS_NO_MORE_ENTRIES)
				break;
			if (status == STATUS_INVALID_PARAMETER)
				break;
			
			if (data_size > 0)
			{
				CHAR* kvfi_ch = new CHAR[data_size];  
				KEY_VALUE_FULL_INFORMATION* kvfi = (KEY_VALUE_FULL_INFORMATION*)kvfi_ch;
				if (kvfi)
				{
					status = ZwEnumerateValueKey(hKey, idx, KeyValueFullInformation, kvfi, data_size, &data_size);
					kprintf(TRACE_INFO, "%d) on enumerate status=0x%08X, type=%d", idx, RtlNtStatusToDosError(status), kvfi->Type);
					if (NT_SUCCESS(status) && (kvfi->Type == REG_SZ))
					{
						PWCH buf;
						ULONG len;
						
						buf = (PWCH)(kvfi_ch + kvfi->DataOffset);
						len = kvfi->DataLength;
					 	wstring data_w(buf, len);
						kprintf(TRACE_INFO, "%ws", data_w.c_str());

					// 	buf = kvfi->Name;
					// 	len = kvfi->NameLength;
					// 	wstring name_w(buf, len);

					// 	kprintf(TRACE_INFO, "%s) Name=%ws, Data=%ws", idx, data_w.c_str(), name_w.c_str());
					}

					delete[] kvfi_ch;
				}
				else
				{
					kprintf(TRACE_INFO, "Empty record");
				}

				idx++;
			}
		}

		ZwClose(hKey);
	}

	kprintf(TRACE_INFO, "exit with status 0x%08X", RtlNtStatusToDosError(status));
	return ok;
}

typedef struct _THREAD_CTX {
	KEVENT evKill;
	PKTHREAD thread;
 } *PTHREAD_CTX, THREAD_CTX;

KEVENT dummy;

VOID ThreadProc(PTHREAD_CTX ctx)
{
	NTSTATUS status;
	PVOID objects[] = { &ctx->evKill, &dummy };

	kprintf(TRACE_INFO, "Registry listener is running");

	ReadPreferences(L"aaa");

	while (TRUE)
	{
		// KeWaitForSingleObject(&ctx->evKill, Executive, KernelMode, FALSE, NULL);
		status = KeWaitForMultipleObjects(RTL_NUMBER_OF(objects), 
			objects, WaitAny, Executive, KernelMode, FALSE, NULL, NULL);

		if (!NT_SUCCESS(status) || status == STATUS_WAIT_0)
			break;

		// WAIT_STATUS_1
		// NTSYSAPI NTSTATUS ZwNotifyChangeKey(
	}

	kprintf(TRACE_INFO, "Registry listener exited with status 0x%08X", status);
	PsTerminateSystemThread(status);
}

NTSTATUS StartThread(PTHREAD_CTX ctx)
{
	NTSTATUS status;
	HANDLE hThread;

	kprintf(TRACE_INFO, "Starting registry listener...");

	// Initialize event for stopping thread
	KeInitializeEvent(&ctx->evKill, NotificationEvent, FALSE);
	KeInitializeEvent(&dummy, NotificationEvent, FALSE);

	// Create thread
	status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS,
	  NULL, NULL, NULL, (PKSTART_ROUTINE) ThreadProc, ctx);
	
	if (!NT_SUCCESS(status))
	  return status;
	
	// Increment object counter
	ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL,
	  KernelMode, (PVOID*) &ctx->thread, NULL);

	ZwClose(hThread);
	kprintf(TRACE_INFO, "Starting registry listener... ok");
	return STATUS_SUCCESS;
}

VOID StopThread(PTHREAD_CTX ctx)
{
	kprintf(TRACE_INFO, "Stopping registry listener...");
	
	KeSetEvent(&ctx->evKill, 0, FALSE);
	KeWaitForSingleObject(ctx->thread, Executive, KernelMode, FALSE, NULL);
	
	ObDereferenceObject(ctx->thread);
	kprintf(TRACE_INFO, "Stopping registry listener... ok");
}




unique_ptr<THREAD_CTX> g_ctx = nullptr;


NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	DrvObject->DriverUnload = [](DRIVER_OBJECT* DriverObject) {
		UNREFERENCED_PARAMETER(DriverObject);
		StopThread(g_ctx.get());
		kprintf(TRACE_INFO, "Driver unloaded");
	};	

	g_ctx = make_unique<THREAD_CTX>();

	StartThread(g_ctx.get());
	kprintf(TRACE_INFO, "Driver is loaded");

	return STATUS_SUCCESS;
}
