#pragma once 

#include <evntrace.h>
#include <functional>

namespace injdrv
{
    using TraceCallback = std::function<void(ULONG pid, ULONG tid, std::wstring data)>;

	VOID TraceStart(TraceCallback callback);

	VOID TraceStop();

}
