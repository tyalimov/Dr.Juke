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

	~CNotificationEvent() 
	{
		if (hEvent)
			ZwClose(hEvent);
	}
};

class RegistryNotifier
{

private:

	HANDLE mKeyHandle = nullptr;
	IO_STATUS_BLOCK mIob;
	LPCWSTR mEvName = L"\\BaseNamedObjects\\TestEvent";
	unique_ptr<CNotificationEvent> mEvNotify;

public:

	RegistryNotifier(LPCWSTR AbsKeyPath)
	{
		NTSTATUS Status;
		UNICODE_STRING KeyPath;
		OBJECT_ATTRIBUTES Attr;

		mEvNotify = make_unique<CNotificationEvent>(mEvName);
		RtlInitUnicodeString(&KeyPath, AbsKeyPath);
		InitializeObjectAttributes(&Attr, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
		
		Status = ZwOpenKeyEx(&mKeyHandle, KEY_NOTIFY, &Attr, 0);
		kprintf(TRACE_INFO, "Exit with status=0x%08X", Status);
	}

	~RegistryNotifier()
	{
		if (mKeyHandle)
			ZwClose(mKeyHandle);
	}

	NTSTATUS WaitForChangesAsync()
	{
		NTSTATUS Status = ZwNotifyChangeKey(mKeyHandle, mEvNotify->hEvent, NULL, 
			(PVOID)DelayedWorkQueue, &mIob, REG_NOTIFY_CHANGE_LAST_SET, FALSE, NULL, 0, TRUE);
		
		kprintf(TRACE_INFO, "Exit with status=0x%08X", Status);
		return Status;
	}

	PKEVENT GetPKEvent() {
		return mEvNotify->pkEvent;
	}

	void print_all()
	{
		kprintf(TRACE_INFO, "Registry change: cnt=%d, status=0x%08X", (ULONG)mIob.Information, mIob.Status);
	}
};

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
	RegistryNotifier notifier(L"\\Registry\\Machine\\SOFTWARE\\RegisteredApplications");	
	
	PKEVENT EvNotify = notifier.GetPKEvent();
	PVOID objects[] = { &ctx->evKill, EvNotify};

	while (TRUE)
	{
		KeClearEvent(EvNotify);
		notifier.WaitForChangesAsync();

		status = KeWaitForMultipleObjects(RTL_NUMBER_OF(objects), 
			objects, WaitAny, Executive, KernelMode, FALSE, NULL, NULL);
	
		if (!NT_SUCCESS(status) || status == STATUS_WAIT_0)
			break;

		// WAIT_STATUS_1
		notifier.print_all();
	}

	PRINT_STATUS(TRACE_THREAD, status);
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
	StartThread(g_ctx.get());
	kprintf(TRACE_INFO, "Driver is loaded");

	return STATUS_SUCCESS;
}
