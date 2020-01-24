#pragma once

#include <ntifs.h>

#define DRIVER_NAME             L"AVKernel"
#define DRIVER_NAME_WITH_EXT    L"AVKernel.sys"
#define NT_DEVICE_NAME          L"\\Device\\AVKernel"

#define OS_WIN8_OR_GREATER     (NTDDI_VERSION >= NTDDI_WIN8)
#define OS_WIN7_OR_GREATER     (NTDDI_VERSION >= NTDDI_WIN7)
#define OS_VISTA_OR_GREATER    (NTDDI_VERSION >= NTDDI_VISTA)

//-------------------------------------------------------------------------------->
// Tracing options

#define TRACE_NONE				0x00000000
#define TRACE_INFO				0x00000001
#define TRACE_ERROR				0x00000002
#define TRACE_REGFILTER			0x00000004
#define TRACE_PREF				0x00000008
#define TRACE_FSFILTER			0x00000010
#define TRACE_OS_VER			0x00000020
#define TRACE_CHANGES			0x00000040
#define TRACE_LOAD				0x00000080

#define TRACE_ALL (	TRACE_NONE \
	| TRACE_INFO \
	| TRACE_ERROR \
	| TRACE_REGFILTER \
	| TRACE_PREF \
	| TRACE_FSFILTER \
	| TRACE_OS_VER \
	| TRACE_CHANGES \
	| TRACE_LOAD)

#define TRACE_CUSTOM (	TRACE_NONE \
	| TRACE_INFO \
	| TRACE_ERROR)

#define TRACE_FLAGS TRACE_ALL 

//-------------------------------------------------------------------------------->
// Print routines

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
