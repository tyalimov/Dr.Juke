#include "rcn.h"
#include "util.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"

using namespace eastl;

struct RCNHandler
{
	LPCWSTR KeyPath;
	void (*Handler)();
};

RCNHandler gRcnHandlers[] = {
	{ KEY_APPS, OnKeyAppsChange },
	{ KEY_TOTALCMD, OnKeyTotalcmdChange },
};

static_assert(RTL_NUMBER_OF(gRcnHandlers) <= 64 - 1, "Wait objects limit exceeded");

class RCNKey
{

private:

	HANDLE mKeyHandle = nullptr;
	unique_ptr<CNotificationObject> mEvNotify;
	NTSTATUS mStatus;

public:

	RCNKey(LPCWSTR AbsKeyPath, LPCWSTR EventName)
	{
		UNICODE_STRING KeyPath;
		OBJECT_ATTRIBUTES Attr;

		mEvNotify = make_unique<CNotificationObject>(EventName);

		RtlInitUnicodeString(&KeyPath, AbsKeyPath);
		InitializeObjectAttributes(&Attr, &KeyPath, OBJ_CASE_INSENSITIVE, NULL, NULL);	
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
		mKeyHandle = other.mKeyHandle;
		mEvNotify = move(other.mEvNotify);
		other.mKeyHandle = nullptr;
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

	~RCNKey() 
	{
		if (mKeyHandle)
			ZwClose(mKeyHandle);
	}
};

typedef struct _THREAD_CTX {
	KEVENT EvKill;
	PKTHREAD PKThread;
} *PTHREAD_CTX, THREAD_CTX;

VOID RCNThreadEntry(PTHREAD_CTX ctx)
{
	NTSTATUS Status;
	PVOID Objects[RTL_NUMBER_OF(gRcnHandlers) + 1] = { 0 };
	Objects[0] = &ctx->EvKill;

	kprintf(TRACE_THREAD, "Registry listener is running");

	WCHAR NumBuf[3] = { 0 };
	vector<RCNKey> NotifyKeys;
	for (int i = 0; i < RTL_NUMBER_OF(gRcnHandlers); i++)
	{
		// Create reg key notificators with unique event names
		// NumBuf may contain values [0..63]
		_itow(i, NumBuf, 10);
		wstring EvName = L"\\BaseNamedObjects\\RCNEvent" + wstring(NumBuf);
		NotifyKeys.push_back(RCNKey(gRcnHandlers[i].KeyPath, EvName.c_str()));

		auto NotifyObject = NotifyKeys[i].GetNotifyObject();
		Status = NotifyKeys[i].GetKeyOpenStatus();

		// Check for errors before continue
		// Skip ones, those have errors
		if (!NT_SUCCESS(Status))
		{
			kprintf(TRACE_ERROR, "Failed to open registry key %ws. Status=0x%08X",
				gRcnHandlers[i].KeyPath, Status);
		}
		else if (NotifyObject->EventHandle == nullptr || NotifyObject->PKEvent == nullptr)
			kprintf(TRACE_ERROR, "Failed to create event %ws", EvName.c_str());
		else
		{
			// Objects[0] is EvKill, so use i + 1
			Objects[i + 1] = NotifyObject->PKEvent;
		}
	}
	
	while (TRUE)
	{
		// Subscribe to registry keys notifications (async)
		for (int i = 0; i < RTL_NUMBER_OF(gRcnHandlers); i++)
		{
			auto NotifyObj = NotifyKeys[i].GetNotifyObject();
			auto KeyHandle = NotifyKeys[i].GetKeyHandle();
			KeClearEvent(NotifyObj->PKEvent);

			Status = ZwNotifyChangeKey(KeyHandle, NotifyObj->EventHandle, NULL, 
				(PVOID)DelayedWorkQueue, NULL, REG_NOTIFY_CHANGE_LAST_SET, FALSE, NULL, 0, TRUE);
		}

		// Wait for registry key change event or EvKill
		Status = KeWaitForMultipleObjects(RTL_NUMBER_OF(Objects), 
			Objects, WaitAny, Executive, KernelMode, FALSE, NULL, NULL);
	
		if (!NT_SUCCESS(Status) || Status == STATUS_WAIT_0)
			break;

		// WAIT_STATUS_1
		LONG j = Status - 1;
		gRcnHandlers[j].Handler();
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

unique_ptr<THREAD_CTX> gThreadCtx;
NTSTATUS RCNInit()
{
	gThreadCtx = make_unique<THREAD_CTX>();
	return RCNStartThread(gThreadCtx.get());
}

void RCNExit()
{
	RCNStopThread(gThreadCtx.get());
}
