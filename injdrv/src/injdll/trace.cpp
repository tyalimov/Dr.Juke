
#include "trace.h"

namespace _internal
{
	REGHANDLE ProviderHandle;
	_snwprintf_fn_t _snwprintf;

	void trace_init(HANDLE NtdllHandle)
	{
		ANSI_STRING RoutineName;
		RtlInitAnsiString(&RoutineName, (PSTR)"_snwprintf");
		LdrGetProcedureAddress(NtdllHandle, &RoutineName, 0, (PVOID*)&_snwprintf);
		RTL_ASSERT(_snwprintf != nullptr);

		const GUID ProviderGuid = { 0xa4b4ba50, 0xa667, 0x43f5,
			{ 0x91, 0x9b, 0x1e, 0x52, 0xa6, 0xd6, 0x9b, 0xd5 } };

		EtwEventRegister(&ProviderGuid,
			NULL,
			NULL,
			&ProviderHandle);
	}

	_snwprintf_fn_t GetSnwprintfPtr() {
		return _snwprintf;
	}

	REGHANDLE GetProviderHandle() {
		return ProviderHandle;
	}

}

