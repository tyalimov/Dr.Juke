#pragma once

#include "common.h"
#include <EASTL/functional.h>
#include <EASTL/string.h>

using PID = HANDLE;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SystemProcessorPerformanceInformation = 8,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45,
	SystemPolicyInformation = 134,
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG           NextEntryOffset;
	ULONG           NumberOfThreads;
	LARGE_INTEGER   Reserved[3];
	LARGE_INTEGER   CreateTime;
	LARGE_INTEGER   UserTime;
	LARGE_INTEGER   KernelTime;
	UNICODE_STRING  ImageName;
	KPRIORITY       BasePriority;
	HANDLE          ProcessId;
	HANDLE          InheritedFromProcessId;
	ULONG           HandleCount;
	UCHAR           Reserved4[4];
	PVOID           Reserved5[11];
	SIZE_T          PeakPagefileUsage;
	SIZE_T          PrivatePageCount;
	LARGE_INTEGER   Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY     LoadOrder;
	LIST_ENTRY     MemoryOrder;
	LIST_ENTRY     InitializationOrder;
	PVOID          ModuleBaseAddress;
	PVOID          EntryPoint;
	ULONG          ModuleSize;
	UNICODE_STRING FullModuleName;
	UNICODE_STRING ModuleName;
	ULONG          Flags;
	USHORT         LoadCount;
	USHORT         TlsIndex;
	union {
		LIST_ENTRY Hash;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		} s;
	} u;
	ULONG   TimeStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef NTSTATUS (*ZwQuerySystemInformationRoutine)(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
);

typedef NTSTATUS (*ZwQueryInformationProcessRoutine)(
	_In_      HANDLE                    ProcessHandle,
	_In_      PROCESSINFOCLASS          ProcessInformationClass,
	_Out_     PVOID                     ProcessInformation,
	_In_      ULONG                     ProcessInformationLength,
	_Out_opt_ PULONG                    ReturnLength
);

// Warning!
// Do not use this class until loaded routines
// ZwQuerySystemInformation and ZwQueryProcessInformation
class ProcessList
{
private:

	PCHAR m_info = nullptr;

	class iterator
	{
	private:
		PSYSTEM_PROCESS_INFORMATION m_info_ptr = nullptr;
		SIZE_T m_offset = 0;

	public:
		iterator(PSYSTEM_PROCESS_INFORMATION info) : m_info_ptr(info) {}
		iterator() = default;

		const PSYSTEM_PROCESS_INFORMATION& operator*() const {
			return m_info_ptr;
		}

		iterator& operator++();

		bool operator==(const iterator& other) const;

		bool operator!=(const iterator& other) const;
	};

public:

	ProcessList()
	{
		NTSTATUS status = getProcessList();
		kprint_st(TRACE_INFO, status);
	}

	~ProcessList() 
	{
		if (m_info)
		{
			delete[] m_info;
			m_info = nullptr;
		}
	}

	// Forbid copy/move
	ProcessList(const ProcessList&) = delete;
	ProcessList(ProcessList&&) = delete;

	iterator begin() const 
	{
		return iterator((PSYSTEM_PROCESS_INFORMATION)m_info);
	}

	iterator end() const
	{
		return iterator(nullptr);
	}

private:

	NTSTATUS getProcessList();
};

NTSTATUS PsMonInit();

VOID PsMonExit();

NTSTATUS QueryProcessImagePath(HANDLE Process, PUNICODE_STRING* ImagePath);

eastl::wstring GetProcessImagePathByPid(PID ProcessId);

