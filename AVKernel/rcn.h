#pragma once

#include <EASTL/vector.h>

using RCNHandler = void(*)();

struct RCNTracker
{
	LPCWSTR KeyPath;
	RCNHandler Handler;
};

typedef struct _THREAD_CTX {
	KEVENT EvKill;
	PKTHREAD PKThread;
	eastl::vector<RCNTracker> Trackers;
} *PTHREAD_CTX, THREAD_CTX, RCN_THREAD_CTX;

NTSTATUS RCNStartThread(PTHREAD_CTX ctx);

void RCNStopThread(PTHREAD_CTX ctx);
