#pragma once

//
// Include NTDLL-related headers.
//
#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>

namespace ownstl
{
	class CriticalSection
	{
	private:

		RTL_CRITICAL_SECTION* m_crit = nullptr;

	public:

		CriticalSection()
		{
			m_crit = new RTL_CRITICAL_SECTION();
			RtlInitializeCriticalSection(m_crit);
		}

		CriticalSection(CriticalSection& other) = delete;

		~CriticalSection()
		{
			if (m_crit != nullptr)
			{
				RtlLeaveCriticalSection(m_crit);
				RtlDeleteCriticalSection(m_crit);
				delete m_crit;

				m_crit = nullptr;
			}
		}

		bool acquire()
		{
			NTSTATUS status = RtlEnterCriticalSection(m_crit);
			return NT_SUCCESS(status);
		}

		bool release()
		{
			NTSTATUS status = RtlLeaveCriticalSection(m_crit);
			return NT_SUCCESS(status);
		}
	};
}
