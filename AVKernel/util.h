#pragma once

#include <ntifs.h>

class GuardedMutex
{
private:

	KGUARDED_MUTEX _mutex;

public:

	VOID acquire();
	VOID release();
	BOOLEAN try_acquire();

	GuardedMutex();
	~GuardedMutex() = default;

	// forbid copy/move
	GuardedMutex(const GuardedMutex&) = delete;
	GuardedMutex(GuardedMutex&&) = delete;
};

template <class TGuard>
class LockGuard
{

public:

	TGuard* _guard;
	LockGuard(TGuard* guard) {
		_guard = guard;
		_guard->acquire();
	}

	~LockGuard() {
		_guard->release();
	}

	// forbid copy/move
	LockGuard(const LockGuard&) = delete;
	LockGuard(LockGuard&&) = delete;
};

struct CNotificationObject
{
	HANDLE EventHandle = nullptr;
	PKEVENT PKEvent = nullptr;

	// forbid copy
	CNotificationObject& operator=(const CNotificationObject& other) = delete;
	CNotificationObject(const CNotificationObject& other) = delete;

	// allow move
	CNotificationObject(CNotificationObject&& other);
	CNotificationObject& operator=(CNotificationObject&& other);

	CNotificationObject(LPCWSTR name);
	~CNotificationObject();
};
