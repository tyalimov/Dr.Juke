#pragma once

#include <ntifs.h>

#include <EASTL/string.h>

using namespace eastl;

wstring GetKeyNameByHandle(HANDLE KeyHandle);

BOOLEAN IsWin8OrGreater();

namespace str_util
{
	bool startsWith(const wstring& str, const wstring& substr);

	bool startsWith(const string& str, const string& substr);

	bool startsWith(const wstring& str, const wchar_t* substr);

	bool startsWith(const string& str, const char* substr);

	bool startsWith(const wstring* str, const wchar_t* substr);

	bool startsWith(const string* str, const char* substr);

	void makeLower(wstring* str);

	void makeLower(string* str);

	bool compareIns(string str1, string str2);

	bool compareIns(wstring str1, wstring str2);

}

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

// struct CNotificationObject
// {
// 	HANDLE EventHandle = nullptr;
// 	PKEVENT PKEvent = nullptr;
// 
// 	// forbid copy
// 	CNotificationObject& operator=(const CNotificationObject& other) = delete;
// 	CNotificationObject(const CNotificationObject& other) = delete;
// 
// 	// allow move
// 	CNotificationObject(CNotificationObject&& other);
// 	CNotificationObject& operator=(CNotificationObject&& other);
// 
// 	CNotificationObject(LPCWSTR name);
// 	~CNotificationObject();
// };
