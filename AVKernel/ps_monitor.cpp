#include "common.h"
#include "util.h"
#include <ntddk.h>
#include "ps_monitor.h"
#include "access_monitor.h"

extern PRegistryAccessMonitor gRegMon;

BOOLEAN ProcessNotifyRoutineSet = FALSE;
extern ZwQuerySystemInformationRoutine ZwQuerySystemInformation;
extern ZwQueryInformationProcessRoutine ZwQueryInformationProcess;

VOID CreateProcessNotifyRoutine(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
    )
{
	UNREFERENCED_PARAMETER(Process);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(CreateInfo);

    if (CreateInfo != NULL)
    {
        //DbgPrint(
        //    "ObCallbackTest: TdCreateProcessNotifyRoutine2: process %p (ID 0x%p) created, creator %Ix:%Ix\n"
        //    "    command line %wZ\n"
        //    "    file name %wZ (FileOpenNameAvailable: %d)\n",
        //    Process,
        //    (PVOID)ProcessId,
        //    (ULONG_PTR)CreateInfo->CreatingThreadId.UniqueProcess,
        //    (ULONG_PTR)CreateInfo->CreatingThreadId.UniqueThread,
        //    CreateInfo->CommandLine,
        //    CreateInfo->ImageFileName,
        //    CreateInfo->FileOpenNameAvailable
        //);

		wstring ImagePath = GetProcessImagePathByPid(ProcessId);
		kprintf(TRACE_INFO, "Create process: %ws", ImagePath.c_str());

		if (gRegMon)
		{
			gRegMon->addProcessIfExcluded(ProcessId, ImagePath);
			kprintf(TRACE_INFO, "addProcessIfExcluded");
		}

    }
    else
    {
		if (gRegMon)
		{
			gRegMon->removeProcessIfExcluded(ProcessId);
			kprintf(TRACE_INFO, "removeProcessIfExcluded");
		}

        //DbgPrint(
        //    "ObCallbackTest: TdCreateProcessNotifyRoutine2: process %p (ID 0x%p) destroyed\n",
        //    Process,
        //    (PVOID)ProcessId
        //);
    }
}

NTSTATUS PsMonInit()
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ProcessNotifyRoutineSet = FALSE;

    kprintf(TRACE_INFO, "Callback version 0x%hx", ObGetFilterVersion());
  
    Status = PsSetCreateProcessNotifyRoutineEx (
        CreateProcessNotifyRoutine,
        FALSE
    );

    if (NT_SUCCESS(Status))
        ProcessNotifyRoutineSet = TRUE;

    kprint_st(TRACE_INFO, Status);
    return Status;
}

VOID PsMonExit()
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if (ProcessNotifyRoutineSet == TRUE)
    {
        Status = PsSetCreateProcessNotifyRoutineEx (
            CreateProcessNotifyRoutine,
            TRUE
        );

        ProcessNotifyRoutineSet = FALSE;
    }

    kprint_st(TRACE_INFO, Status);
}

ProcessList::iterator& ProcessList::iterator::operator++()
{
	m_offset = m_info_ptr->NextEntryOffset;
	if (m_offset)
		m_info_ptr = (PSYSTEM_PROCESS_INFORMATION)((SIZE_T)m_info_ptr + m_offset);
	else
		m_info_ptr = nullptr;

	return *this;
}

bool ProcessList::iterator::operator==(const iterator& other) const
{
	return m_info_ptr == other.m_info_ptr &&
		m_offset == other.m_offset;
}

bool ProcessList::iterator::operator!=(const iterator& other) const
{
	return !(*this == other);
}

NTSTATUS ProcessList::getProcessList()
{
	NTSTATUS status;
	ULONG size = 0, written = 0;

	// Query required size
	status = ZwQuerySystemInformation(SystemProcessInformation, 0, 0, &size);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return status;

	while (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		// We should allocate little bit more space
		size += written; 
		
		if (m_info)
			delete[] m_info;

		m_info = new CHAR[size];
		if (!m_info)
			break;

		status = ZwQuerySystemInformation(SystemProcessInformation, m_info, size, &written);
	}

	if (!m_info)
		return STATUS_ACCESS_DENIED;

	if (!NT_SUCCESS(status))
	{
		delete[] m_info;
		m_info = nullptr;
	}

	return status;
}

NTSTATUS QueryProcessImagePath(HANDLE Process, PUNICODE_STRING* ImagePath)
{
	NTSTATUS status;
	PCHAR info = nullptr;
	ULONG size = 0, written = 0;

	// Query required size
	status = ZwQueryInformationProcess(Process, ProcessImageFileName, 0, 0, &size);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return status;

	while (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		// We should allocate little bit more space
		size += written;

		if (info)
			delete[] info;

		info = new CHAR[size]; 
		if (!info)
			break;

		status = ZwQueryInformationProcess(Process, ProcessImageFileName, info, size, &written);
	}

	if (!info)
		return status;

	if (!NT_SUCCESS(status))
	{
		delete[] info;
		info = nullptr;
	}

	*ImagePath = (PUNICODE_STRING)info;
	return status;
}

wstring GetProcessImagePathByPid(PID ProcessId)
{
	NTSTATUS status;
	HANDLE hProcess;
	CLIENT_ID clientId;
	OBJECT_ATTRIBUTES attribs;
	PUNICODE_STRING ImagePath;
	wstring res;

	InitializeObjectAttributes(&attribs, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	clientId.UniqueProcess = ProcessId;
	clientId.UniqueThread = 0;

	status = ZwOpenProcess(&hProcess, 0x1000/*PROCESS_QUERY_LIMITED_INFORMATION*/, &attribs, &clientId);
	if (!NT_SUCCESS(status))
	{
		kprintf(TRACE_ERROR, "Can't open process (pid:%p) failed with code:%08x", ProcessId, status);
		return res;
	}

	status = QueryProcessImagePath(hProcess, &ImagePath);
	if (NT_SUCCESS(status) && ImagePath != nullptr)
		res = wstring(ImagePath->Buffer, ImagePath->Length);
	
	if (ImagePath)
		delete[] (PCHAR)ImagePath;

	ZwClose(hProcess);
	return res;
}
