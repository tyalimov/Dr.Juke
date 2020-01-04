#include "util.h"
#include "common.h"

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
