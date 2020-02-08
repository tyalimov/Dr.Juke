#pragma once

typedef int (*_snwprintf_fn_t)(wchar_t* buffer,
	size_t count, const wchar_t* format, ...);

#define TRACE_NONE				0x00000000
#define TRACE_INFO				0x00000001
#define TRACE_WARN				0x00000002

//
// Include support for ETW logging.
// Note that following functions are mocked, because they're
// located in advapi32.dll.  Fortunatelly, advapi32.dll simply
// redirects calls to these functions to the ntdll.dll.
//

#define EventActivityIdControl  EtwEventActivityIdControl
#define EventEnabled            EtwEventEnabled
#define EventProviderEnabled    EtwEventProviderEnabled
#define EventRegister           EtwEventRegister
#define EventSetInformation     EtwEventSetInformation
#define EventUnregister         EtwEventUnregister
#define EventWrite              EtwEventWrite
#define EventWriteEndScenario   EtwEventWriteEndScenario
#define EventWriteEx            EtwEventWriteEx
#define EventWriteStartScenario EtwEventWriteStartScenario
#define EventWriteString        EtwEventWriteString
#define EventWriteTransfer      EtwEventWriteTransfer

#include <ntdll.h>
#include <evntprov.h>

namespace _internal
{
	void trace_init(HANDLE NtdllHandle);
	_snwprintf_fn_t GetSnwprintfPtr();
	REGHANDLE GetProviderHandle();
}

class Trace
{
	static constexpr int buf_size = 512;
	static constexpr bool info_enabled = true;
	static constexpr bool err_enabled = true;
	static constexpr bool warn_enabled = true;

	struct unused_parameter 
	{
		template<typename ...Args> 
		unused_parameter(Args const & ... ) {} 
	};

public:

	static void init(HANDLE NtdllHandle) {
		_internal::trace_init(NtdllHandle);
	}

	template <typename ...T>
	static void log(const wchar_t* prefix, const wchar_t* fmt, T... args)
	{
		REGHANDLE ProviderHandle = _internal::GetProviderHandle();
		_snwprintf_fn_t _snwprintf = _internal::GetSnwprintfPtr();

		if (ProviderHandle == 0 || _snwprintf == nullptr)
			return;

		wchar_t buf[buf_size] = { 0 };
		size_t prefix_len = wcslen(prefix);
		_snwprintf(buf, RTL_NUMBER_OF(buf), prefix);
		_snwprintf(buf + prefix_len, RTL_NUMBER_OF(buf) - prefix_len, fmt, args...);

		EtwEventWriteString(ProviderHandle, 0, 0, buf);
	}

	template <typename ...T>
	inline static void logInfo(const wchar_t* fmt, T... args)
	{
		if constexpr (info_enabled)
			log(L"[INFO] ", fmt, args...);
		else
			unused_parameter(args...);
	}

	template <typename ...T>
	inline static void logWarning(const wchar_t* fmt, T... args)
	{
		if constexpr (info_enabled)
			log(L"[WARNING] ", fmt, args...);
		else
			unused_parameter(args...);
	}

	template <typename ...T>
	inline static void logError(const wchar_t* fmt, T... args)
	{
		if constexpr (info_enabled)
			log(L"[ERROR] ", fmt, args...);
		else
			unused_parameter(args...);
	}
};

