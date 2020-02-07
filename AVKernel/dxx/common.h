#pragma once

#include <ntifs.h>
#include <EASTL/string.h>

#define DRIVER_NAME             L"AVKernel"
#define DRIVER_NAME_WITH_EXT    L"dxx.sys"
#define NT_DEVICE_NAME          L"\\Device\\AVKernel"

#define OS_WIN8_OR_GREATER     (NTDDI_VERSION >= NTDDI_WIN8)
#define OS_WIN7_OR_GREATER     (NTDDI_VERSION >= NTDDI_WIN7)
#define OS_VISTA_OR_GREATER    (NTDDI_VERSION >= NTDDI_VISTA)

#define DR_JUKE_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke"

#define REGFILTER L"REG_FILTER"
#define FSFILTER L"FS_FILTER"
#define PSMONITOR L"PS_MONITOR"

//-------------------------------------------------------------------------------->
// Tracing options

//#define TRACING_STANDARD
//#define TRACING_CUSTOM
//#define TRACING_FSFILTER_ONLY
//#define TRACING_REGFILTER_ONLY
//#define TRACING_PSPROTECT_ONLY
#define TRACING_NETFILTER_ONLY


#define TRACE_NONE				0x00000000
#define TRACE_INFO				0x00000001
#define TRACE_WARN				0x00000002
#define TRACE_ERROR				0x00000004
#define TRACE_PSMON				0x00000008
#define TRACE_PREF				0x00000010
#define TRACE_PSLIST			0x00000020
#define TRACE_FSFILTER			0x00000040
#define TRACE_REGFILTER			0x00000080
#define TRACE_PSPROTECT			0x00000100
#define TRACE_NETFILTER_INFO	0x00000200
#define TRACE_NETFILTER_WARN	0x00000400
#define TRACE_NETFILTER_ERROR	0x00000800

#define TRACE_STANDARD ( TRACE_NONE \
	| TRACE_INFO \
	| TRACE_WARN \
	| TRACE_ERROR)

#define TRACE_REGFILTER_ONLY ( TRACE_NONE \
	| TRACE_REGFILTER \
	| TRACE_STANDARD)

#define TRACE_FSFILTER_ONLY ( TRACE_NONE \
	| TRACE_FSFILTER \
	| TRACE_STANDARD)

#define TRACE_PSPROTECT_ONLY ( TRACE_NONE \
	| TRACE_PSPROTECT \
	| TRACE_STANDARD)

#define TRACE_NETFILTER_ONLY ( TRACE_NONE \
	| TRACE_NETFILTER_INFO \
	| TRACE_NETFILTER_WARN \
	| TRACE_NETFILTER_ERROR \
	| TRACE_STANDARD)


#define TRACE_CUSTOM ( TRACE_NONE \
	| TRACE_INFO \
	| TRACE_WARN \
	| TRACE_ERROR \
	| TRACE_PSMON)

#define LOG_MODE_REGFILTER_CUSTOM LogMode::OFF, LogMode::ON, LogMode::ON
#define LOG_MODE_PSPROTECT_CUSTOM LogMode::OFF, LogMode::ON, LogMode::ON
#define LOG_MODE_FSFILTER_CUSTOM LogMode::OFF, LogMode::ON, LogMode::ON

#ifdef TRACING_STANDARD
#	define LOG_MODE_REGFILTER LogMode::OFF, LogMode::ON, LogMode::ON
#	define LOG_MODE_PSPROTECT LogMode::OFF, LogMode::ON, LogMode::ON
#	define LOG_MODE_FSFILTER LogMode::OFF, LogMode::ON, LogMode::ON
#	define TRACE_FLAGS TRACE_STANDARD
#elif defined(TRACING_CUSTOM)
#	define LOG_MODE_REGFILTER LOG_MODE_REGFILTER_CUSTOM
#	define LOG_MODE_PSPROTECT LOG_MODE_PSPROTECT_CUSTOM 
#	define LOG_MODE_FSFILTER LOG_MODE_FSFILTER_CUSTOM
#	define TRACE_FLAGS TRACE_CUSTOM
#elif defined(TRACING_FSFILTER_ONLY)
#	define LOG_MODE_REGFILTER LogMode::OFF, LogMode::OFF, LogMode::OFF
#	define LOG_MODE_PSPROTECT LogMode::OFF, LogMode::OFF, LogMode::OFF 
#	define LOG_MODE_FSFILTER LogMode::ON, LogMode::ON, LogMode::ON
#	define TRACE_FLAGS TRACE_FSFILTER_ONLY
#elif defined(TRACING_REGFILTER_ONLY)
#	define LOG_MODE_REGFILTER LogMode::ON, LogMode::ON, LogMode::ON
#	define LOG_MODE_PSPROTECT LogMode::OFF, LogMode::OFF, LogMode::OFF 
#	define LOG_MODE_FSFILTER LogMode::OFF, LogMode::OFF, LogMode::OFF
#	define TRACE_FLAGS TRACE_REGFILTER_ONLY
#elif defined(TRACING_PSPROTECT_ONLY)
#	define LOG_MODE_REGFILTER LogMode::OFF, LogMode::OFF, LogMode::OFF
#	define LOG_MODE_PSPROTECT LogMode::ON, LogMode::ON, LogMode::ON 
#	define LOG_MODE_FSFILTER LogMode::OFF, LogMode::OFF, LogMode::OFF
#	define TRACE_FLAGS TRACE_PSPROTECT_ONLY
#elif defined(TRACING_NETFILTER_ONLY)
#	define LOG_MODE_REGFILTER LogMode::OFF, LogMode::OFF, LogMode::OFF
#	define LOG_MODE_PSPROTECT LogMode::OFF, LogMode::OFF, LogMode::OFF 
#	define LOG_MODE_FSFILTER LogMode::OFF, LogMode::OFF, LogMode::OFF
#	define TRACE_FLAGS TRACE_NETFILTER_ONLY
#else
#	error "Need to specify tracing mode"
#endif

//-------------------------------------------------------------------------------->
// Print routines

#define kprintf(level, fmt, ...)			\
    (FlagOn(TRACE_FLAGS,(level)) ?			\
        DbgPrint("[%s] %S!%s: "##fmt, #level, DRIVER_NAME, __FUNCTION__, __VA_ARGS__) : ((int)0))

#define kprint_st(level, status)		\
    (FlagOn(TRACE_FLAGS,(level)) ?		\
        DbgPrint("[%s] %S!%s: %s 0x%08X", #level, DRIVER_NAME, __FUNCTION__, "Exit with status", status) : ((int)0))

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
