#pragma once

#include <ntifs.h>
#include <EASTL/string.h>

#define DRIVER_NAME             L"AVKernel"
#define DRIVER_NAME_WITH_EXT    L"AVKernel.sys"
#define NT_DEVICE_NAME          L"\\Device\\AVKernel"

#define OS_WIN8_OR_GREATER     (NTDDI_VERSION >= NTDDI_WIN8)
#define OS_WIN7_OR_GREATER     (NTDDI_VERSION >= NTDDI_WIN7)
#define OS_VISTA_OR_GREATER    (NTDDI_VERSION >= NTDDI_VISTA)

#define DR_JUKE_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke"

#define PIPEFILTER L"PIPE_FILTER"
#define REGFILTER L"REG_FILTER"
#define FSFILTER L"FS_FILTER"
#define PSMONITOR L"PS_MONITOR"

//-------------------------------------------------------------------------------->
// Tracing options

#define TRACE_NONE				0x00000000
#define TRACE_INFO				0x00000001
#define TRACE_WARN				0x00000002
#define TRACE_ERROR				0x00000004
#define TRACE_PSMON				0x00000008
#define TRACE_PREF				0x00000010
#define TRACE_PSLIST			0x00000020
#define TRACE_LOAD				0x00000040

#define TRACE_ALL (	TRACE_NONE \
	| TRACE_INFO \
	| TRACE_WARN \
	| TRACE_ERROR \
	| TRACE_PREF \
	| TRACE_PSLIST \
	| TRACE_LOAD)

#define TRACE_STANDARD ( TRACE_NONE \
	| TRACE_INFO \
	| TRACE_WARN \
	| TRACE_ERROR \
	| TRACE_LOAD)

#define TRACE_MINIMAL ( TRACE_NONE \
	| TRACE_WARN \
	| TRACE_ERROR \
	| TRACE_LOAD)

#define TRACE_CUSTOM ( TRACE_NONE \
	| TRACE_INFO \
	| TRACE_WARN \
	| TRACE_ERROR \
	| TRACE_PSMON \
	| TRACE_LOAD)

#define TRACE_FLAGS TRACE_STANDARD

//-------------------------------------------------------------------------------->
// Print routines

#define kprintf(level, fmt, ...)						\
    if constexpr((bool)FlagOn(TRACE_FLAGS,(level)))		\
        DbgPrint("[%s] %S!%s: "##fmt, #level, DRIVER_NAME, __FUNCTION__, __VA_ARGS__)

#define kprint_st(level, status)						\
    if constexpr((bool)FlagOn(TRACE_FLAGS,(level))) 	\
        DbgPrint("[%s] %S!%s: Exit with status 0x%08X", #level, DRIVER_NAME, __FUNCTION__, status)

//-------------------------------------------------------------------------------->
// Class log routines

class FilterPrefixBase
{
public:
	virtual const wchar_t* filter_name() {
		return NULL;
	}

	virtual const wchar_t* driver_name() {
		return DRIVER_NAME;
	}
};

class FsFilterPrefix : public FilterPrefixBase 
{
public:
	virtual const wchar_t* filter_name() override {
		return FSFILTER;
	}
};

class RegFilterPrefix : public FilterPrefixBase 
{
public:
	virtual const wchar_t* filter_name() override {
		return REGFILTER;
	}
};

class PipeFilterPrefix : public FilterPrefixBase 
{
public:
	virtual const wchar_t* filter_name() override {
		return PIPEFILTER;
	}
};

class PsMonitorPrefix : public FilterPrefixBase 
{
public:
	virtual const wchar_t* filter_name() override {
		return PSMONITOR;
	}
};

enum class LogMode { ON, OFF };

template <typename TFilter>
class FilterLog
{
public:

	template<typename ...T>
	static void log(const char* log_type, const char* fmt, T... args)
	{
		TFilter filter;
		eastl::string fullfmt = eastl::string("[%ws][%ws_%s]: ") + fmt;
		DbgPrint(fullfmt.c_str(), filter.driver_name(), filter.filter_name(), log_type, args...);
	}

	template<typename ...T>
	static inline void logInfo(const char* fmt, T... args) {
		log("INFO", fmt, args...);
	}

	template<typename ...T>
	static inline void logWarning(const char* fmt, T... args) {
		log("WARNING", fmt, args...);
	}

	template<typename ...T>
	static inline void logError(const char* fmt, T... args) {
		log("ERROR", fmt, args...);
	}

};


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
