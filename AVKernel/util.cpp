#include "util.h"
#include "common.h"
#include <EASTL/utility.h>

VOID GuardedMutex::acquire() {
	KeAcquireGuardedMutex(&_mutex);
}

VOID GuardedMutex::release() {
	KeReleaseGuardedMutex(&_mutex);
}

BOOLEAN GuardedMutex::try_acquire() {
	return KeTryToAcquireGuardedMutex(&_mutex);
}

GuardedMutex::GuardedMutex() {
	KeInitializeGuardedMutex(&_mutex);
}

CNotificationObject::CNotificationObject(LPCWSTR name)
{
	UNICODE_STRING us;
	RtlInitUnicodeString(&us, name);
	PKEvent = IoCreateNotificationEvent(&us, &EventHandle);
	KeClearEvent(PKEvent);

	kprintf(TRACE_NOTIFY_OBJ, "EventName=%ws, pkEvent=0x%p, hEvent=0x%p", name, PKEvent, EventHandle);
}

CNotificationObject::~CNotificationObject() 
{
	if (EventHandle)
		ZwClose(EventHandle);
}

CNotificationObject::CNotificationObject(CNotificationObject&& other) {
	*this = eastl::move(other);
}

CNotificationObject& CNotificationObject::operator=(CNotificationObject&& other)
{
	EventHandle = other.EventHandle;
	PKEvent = other.PKEvent;
	other.EventHandle = nullptr;
	other.PKEvent = nullptr;
	return *this;
}
