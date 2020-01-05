#pragma once

#include "drv_info.h"


#define kprintf(level, fmt, ...)			\
    (FlagOn(TRACE_FLAGS,(level)) ?			\
        DbgPrint("[%s] %S!%s: "##fmt, #level, DRIVER_NAME, __FUNCTION__, __VA_ARGS__) : ((int)0))

#define PRINT_STATUS(level, status) \
	kprintf(level, "Exit with status 0x%08X", status);

//#include <EASTL/string.h>
//#include <EASTL/vector.h>
//#include <EASTL/sort.h>
//#include <EASTL/unique_ptr.h>
//#include <EASTL/shared_ptr.h>
//#include <EASTL/scoped_ptr.h>
//#include <EASTL/set.h>
//#include <EASTL/map.h>
//#include <EASTL/unordered_set.h>
//#include <EASTL/unordered_map.h>
//#include "kcrt/path.h"
