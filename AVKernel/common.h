#pragma once

#include "drv_info.h"
#include <ntifs.h>


#define kprintf(level, fmt, ...)			\
    (FlagOn(TRACE_FLAGS,(level)) ?			\
        DbgPrint("[%s] %S!%s: "##fmt, #level, DRIVER_NAME, __FUNCTION__, __VA_ARGS__) : ((int)0))

#define kprint_st(level, status)		\
    (FlagOn(TRACE_FLAGS,(level)) ?		\
        DbgPrint("[%s] %S!%s: %s 0x%08X", #level, DRIVER_NAME, __FUNCTION__, "Exit with status", status) : ((int)0))

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
