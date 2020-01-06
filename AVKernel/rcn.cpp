#include "common.h"
#include "util.h"
#include "rcn.h"

#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"

using namespace eastl;


class RCNKey
{

private:

	unique_ptr<CNotificationObject> mEvNotify = nullptr;
	RCNHandler mHandler = nullptr;
	HANDLE mKeyHandle = nullptr;
	NTSTATUS mStatus;

public:

	RCNKey(const RCNTracker& Tracker, LPCWSTR EventName)
	{
		UNICODE_STRING usKeyPath;
		OBJECT_ATTRIBUTES Attr;

		mHandler = Tracker.Handler;
		mEvNotify = make_unique<CNotificationObject>(EventName);

		RtlInitUnicodeString(&usKeyPath, Tracker.KeyPath);
		InitializeObjectAttributes(&Attr, &usKeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);	
		mStatus = ZwOpenKeyEx(&mKeyHandle, KEY_NOTIFY, &Attr, 0);
	}

	// forbid copy
	RCNKey& operator=(const RCNKey& other) = delete;
	RCNKey(const RCNKey& other) = delete;

	// allow move
	RCNKey(RCNKey&& other) {
		*this = move(other);
	}

	RCNKey& operator=(RCNKey&& other)
	{
		mStatus = other.mStatus;
		mHandler = other.mHandler;
		mKeyHandle = other.mKeyHandle;
		mEvNotify = move(other.mEvNotify);
		other.mKeyHandle = nullptr;
		other.mHandler = nullptr;
		return *this;
	}

	NTSTATUS GetKeyOpenStatus() {
		return mStatus;
	}

	const CNotificationObject* GetNotifyObject() const {
		return mEvNotify.get();
	}

	const HANDLE GetKeyHandle() const {
		return mKeyHandle;
	}

	void InvokeHandler() const 
	{	
		if (mHandler)
			mHandler();
	}

	~RCNKey() 
	{
		if (mKeyHandle)
			ZwClose(mKeyHandle);
	}
};

VOID RCNThreadEntry(PTHREAD_CTX ctx)
{
	int i = 0;
	NTSTATUS Status;
	WCHAR NumBuf[3] = { 0 };
	vector<RCNKey> NotifyKeys;
	vector<PVOID> NotifyObjects;

	kprintf(TRACE_THREAD, "Registry listener is running");

	for (const auto& tracker: ctx->Trackers)
	{
		// Create reg key notifiers with unique event names
		// NumBuf may contain values [0..63]
		_itow(i++, NumBuf, 10);
		wstring EvName = L"\\BaseNamedObjects\\RCNEvent" + wstring(NumBuf);
		RCNKey key(tracker, EvName.c_str());

		const CNotificationObject* NotifyObject = key.GetNotifyObject();
		Status = key.GetKeyOpenStatus();

		// Skip ones, those have errors
		if (!NT_SUCCESS(Status))
		{
			kprintf(TRACE_ERROR, "Failed to open registry key %ws. Status=0x%08X",
				tracker.KeyPath, Status);
		}
		else if (NotifyObject->EventHandle == nullptr || NotifyObject->PKEvent == nullptr)
			kprintf(TRACE_ERROR, "Failed to create event %ws", EvName.c_str());
		else
		{
			NotifyObjects.push_back(NotifyObject->PKEvent);
			NotifyKeys.push_back(move(key));
		}
	}
	
	NotifyObjects.push_back(&ctx->EvKill);

	while (TRUE)
	{
		// Subscribe to registry keys notifications (async)
		for (const auto& key: NotifyKeys)
		{
			auto NotifyObj = key.GetNotifyObject();
			auto KeyHandle = key.GetKeyHandle();

			KeClearEvent(NotifyObj->PKEvent);
			Status = ZwNotifyChangeKey(KeyHandle, NotifyObj->EventHandle, NULL, 
				(PVOID)DelayedWorkQueue, NULL, REG_NOTIFY_CHANGE_LAST_SET, FALSE, NULL, 0, TRUE);
		}

		// Wait for registry key change event or EvKill
		Status = KeWaitForMultipleObjects((ULONG)NotifyObjects.size(), 
			NotifyObjects.data(), WaitAny, Executive, KernelMode, FALSE, NULL, NULL);
	
		if (!NT_SUCCESS(Status))
			break;

		const eastl_size_t j = Status;
		if (j < NotifyKeys.size())
			NotifyKeys[j].InvokeHandler();
		else
		{
			Status = STATUS_SUCCESS;
			break;
		}
	}

	kprint_st(TRACE_THREAD, Status);
	PsTerminateSystemThread(Status);
}

NTSTATUS RCNStartThread(PTHREAD_CTX ctx)
{
	NTSTATUS status;
	HANDLE hThread;

	kprintf(TRACE_THREAD, "Starting registry listener...");

	// Initialize event for stopping thread
	KeInitializeEvent(&ctx->EvKill, NotificationEvent, FALSE);

	// Create thread
	status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS,
	  NULL, NULL, NULL, (PKSTART_ROUTINE) RCNThreadEntry, ctx);
	
	if (!NT_SUCCESS(status))
		return status;
	
	// Increment object counter
	ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL,
	  KernelMode, (PVOID*) &ctx->PKThread, NULL);

	ZwClose(hThread);
	kprintf(TRACE_THREAD, "Starting registry listener... ok");
	return STATUS_SUCCESS;
}

VOID RCNStopThread(PTHREAD_CTX ctx)
{
	kprintf(TRACE_THREAD, "Stopping registry listener...");
	
	KeSetEvent(&ctx->EvKill, 0, FALSE);
	KeWaitForSingleObject(ctx->PKThread, Executive, KernelMode, FALSE, NULL);
	
	ObDereferenceObject(ctx->PKThread);
	kprintf(TRACE_THREAD, "Stopping registry listener... ok");
}

