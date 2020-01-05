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


struct CNotificationEvent
{
	HANDLE hEvent = nullptr;
	PKEVENT pkEvent = nullptr;

	CNotificationEvent(LPCWSTR name)
	{
		UNICODE_STRING us;
		RtlInitUnicodeString(&us, name);
		pkEvent = IoCreateNotificationEvent(&us, &hEvent);
		kprintf(TRACE_INFO, "EventName=%ws, pkEvent=0x%p, hEvent=0x%p", name, pkEvent, hEvent);
		KeClearEvent(pkEvent);
		LONG state = KeResetEvent(pkEvent);
		kprintf(TRACE_INFO, "state=%d", state);
	}

	~CNotificationEvent() {
		ZwClose(hEvent);
	}
};

unique_ptr<CNotificationEvent> g_nf;

NTSTATUS SetupKeyValueChangeNotifier(LPCWSTR reg_path, HANDLE hEvent, PIO_STATUS_BLOCK pio)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING KeyPath;
	HANDLE hKey;
	
	UNREFERENCED_PARAMETER(reg_path);
	UNREFERENCED_PARAMETER(hEvent);

	RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\SOFTWARE\\RegisteredApplications");
	InitializeObjectAttributes(&oa, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKeyEx(&hKey, KEY_NOTIFY, &oa, 0);

	if (NT_SUCCESS(status))
	{
		status = ZwNotifyChangeKey(hKey, g_nf->hEvent, NULL, (PVOID)DelayedWorkQueue, 
			pio, REG_NOTIFY_CHANGE_LAST_SET, FALSE, NULL, 0, TRUE);

		kprintf(TRACE_INFO, "status=0x%08X signaled state=%d", status, KeReadStateEvent(g_nf->pkEvent));
		status = KeWaitForSingleObject(g_nf->pkEvent, Executive, KernelMode, FALSE, NULL);
		kprintf(TRACE_INFO, "after wait status=0x%08X", status);
		while (1);

		ZwClose(hKey);
	}

	kprintf(TRACE_INFO, "Exit with status 0x%08X", status);
	return status;
}

NTSTATUS ReadPreferences(LPCWSTR reg_path)
{
	NTSTATUS status;
	HANDLE hKey;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING KeyPath;
	ULONG DataSize;
	ULONG idx = 0;

	UNREFERENCED_PARAMETER(reg_path);

	//HKEY_LOCAL_MACHINE\SOFTWARE\RegisteredApplications
	RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\SOFTWARE\\RegisteredApplications");
	InitializeObjectAttributes(&oa, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKeyEx(&hKey, KEY_READ, &oa, 0);

	if (NT_SUCCESS(status))
	{
		while (1)
		{
			DataSize = 0;
			status = ZwEnumerateValueKey(hKey, idx, KeyValueFullInformation, NULL, 0, &DataSize);
			if (status == STATUS_NO_MORE_ENTRIES)
			{
				status = STATUS_SUCCESS;
				break;
			}

			if (!NT_SUCCESS(status))
				break;

			if (DataSize > 0)
			{
				CHAR* kvfi_ch = new CHAR[DataSize];
				KEY_VALUE_FULL_INFORMATION* kvfi = (KEY_VALUE_FULL_INFORMATION*)kvfi_ch;
				if (kvfi)
				{
					status = ZwEnumerateValueKey(hKey, idx, KeyValueFullInformation, kvfi, DataSize, &DataSize);
					if (NT_SUCCESS(status) && (kvfi->Type == REG_SZ))
					{
						PWCH buf;
						ULONG len;

						buf = (PWCH)(kvfi_ch + kvfi->DataOffset);
						len = kvfi->DataLength;
						wstring data_w(buf, len);
						kprintf(TRACE_PREF, "%ws", data_w.c_str());

						// 	buf = kvfi->Name;
						// 	len = kvfi->NameLength;
						// 	wstring name_w(buf, len);

						// 	kprintf(TRACE_INFO, "%s) Name=%ws, Data=%ws", idx, data_w.c_str(), name_w.c_str());
					}

					delete[] kvfi_ch;
				}
				else
					kprintf(TRACE_PREF, "Empty record");

				idx++;
			}
		}

		ZwClose(hKey);
	}

	kprintf(TRACE_PREF, "Read %d entries. Exit with status 0x%08X", idx, status);
	return status;
}


typedef struct _THREAD_CTX {
	KEVENT evKill;
	PKTHREAD thread;
 } *PTHREAD_CTX, THREAD_CTX;

VOID ThreadProc(PTHREAD_CTX ctx)
{
	NTSTATUS status;

	kprintf(TRACE_THREAD, "Registry listener is running");

	//ReadPreferences(L"aaa");
	IO_STATUS_BLOCK iob = { 0 };
	status = SetupKeyValueChangeNotifier(L"aaa", g_nf->hEvent, &iob);
	if (status != STATUS_PENDING)
		goto exit;

	PVOID objects[] = { &ctx->evKill, g_nf->pkEvent };
	int i = 0;
	while (TRUE)
	{
		status = KeWaitForMultipleObjects(RTL_NUMBER_OF(objects), 
			objects, WaitAny, Executive, KernelMode, FALSE, NULL, NULL);
	
		if (!NT_SUCCESS(status) || status == STATUS_WAIT_0)
			break;

		if (i++ < 100)
		{
			kprintf(TRACE_INFO, "Registry change: cnt=%d, status=0x%08X", (ULONG)iob.Information, iob.Status);
			status = SetupKeyValueChangeNotifier(L"aaa", g_nf->hEvent, &iob);
			kprintf(TRACE_INFO, "status=0x%08X", status);
		}
		// WAIT_STATUS_1
		// NTSYSAPI NTSTATUS ZwNotifyChangeKey(
	}

exit:
	kprintf(TRACE_THREAD, "Registry listener exited with status 0x%08X", status);
	PsTerminateSystemThread(status);
}

NTSTATUS StartThread(PTHREAD_CTX ctx)
{
	NTSTATUS status;
	HANDLE hThread;

	kprintf(TRACE_THREAD, "Starting registry listener...");

	// Initialize event for stopping thread
	KeInitializeEvent(&ctx->evKill, NotificationEvent, FALSE);

	// Create thread
	status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS,
	  NULL, NULL, NULL, (PKSTART_ROUTINE) ThreadProc, ctx);
	
	if (!NT_SUCCESS(status))
		return status;
	
	// Increment object counter
	ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL,
	  KernelMode, (PVOID*) &ctx->thread, NULL);

	ZwClose(hThread);
	kprintf(TRACE_THREAD, "Starting registry listener... ok");
	return STATUS_SUCCESS;
}

VOID StopThread(PTHREAD_CTX ctx)
{
	kprintf(TRACE_THREAD, "Stopping registry listener...");
	
	KeSetEvent(&ctx->evKill, 0, FALSE);
	KeWaitForSingleObject(ctx->thread, Executive, KernelMode, FALSE, NULL);
	
	ObDereferenceObject(ctx->thread);
	kprintf(TRACE_THREAD, "Stopping registry listener... ok");
}




unique_ptr<THREAD_CTX> g_ctx;


NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	DrvObject->DriverUnload = [](DRIVER_OBJECT* DriverObject) {
		UNREFERENCED_PARAMETER(DriverObject);
		StopThread(g_ctx.get());
		kprintf(TRACE_INFO, "Driver unloaded");
	};	

	g_ctx = make_unique<THREAD_CTX>();
	g_nf = make_unique<CNotificationEvent>( L"\\BaseNamedObjects\\TestEvent");
	

	StartThread(g_ctx.get());
	kprintf(TRACE_INFO, "Driver is loaded");

	return STATUS_SUCCESS;
}
