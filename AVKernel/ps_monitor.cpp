#include "common.h"
#include "util.h"
#include <ntddk.h>
#include "ps_monitor.h"

BOOLEAN ProcessNotifyRoutineSet = FALSE;
ZwQuerySystemInformationRoutine ZwQuerySystemInformation = nullptr;
ZwQueryInformationProcessRoutine ZwQueryInformationProcess = nullptr;

VOID CreateProcessNotifyRoutine(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
    )
{
   // NTSTATUS Status = STATUS_SUCCESS;

    if (CreateInfo != NULL)
    {
        DbgPrint(
            "ObCallbackTest: TdCreateProcessNotifyRoutine2: process %p (ID 0x%p) created, creator %Ix:%Ix\n"
            "    command line %wZ\n"
            "    file name %wZ (FileOpenNameAvailable: %d)\n",
            Process,
            (PVOID)ProcessId,
            (ULONG_PTR)CreateInfo->CreatingThreadId.UniqueProcess,
            (ULONG_PTR)CreateInfo->CreatingThreadId.UniqueThread,
            CreateInfo->CommandLine,
            CreateInfo->ImageFileName,
            CreateInfo->FileOpenNameAvailable
        );

        PCUNICODE_STRING us = CreateInfo->ImageFileName;
        wstring ws(us->Buffer, us->Length);
        const wchar_t* prefix = L"\\??\\";
        if (str_util::startsWith(ws, prefix))
        {
            ws.erase(0, RTL_NUMBER_OF(prefix));
        }

    }
    else
    {
        DbgPrint(
            "ObCallbackTest: TdCreateProcessNotifyRoutine2: process %p (ID 0x%p) destroyed\n",
            Process,
            (PVOID)ProcessId
        );
    }
}

NTSTATUS PsMonInit()
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ProcessNotifyRoutineSet = FALSE;

	UNICODE_STRING routine1 = RTL_CONSTANT_STRING(L"ZwQuerySystemInformation");
	UNICODE_STRING routine2 = RTL_CONSTANT_STRING(L"ZwQueryInformationProcess");
	
	ZwQuerySystemInformation = (ZwQuerySystemInformationRoutine)MmGetSystemRoutineAddress(&routine1);
	if (!ZwQuerySystemInformation)
		goto exit;

	ZwQueryInformationProcess = (ZwQueryInformationProcessRoutine)MmGetSystemRoutineAddress(&routine2);
	if (!ZwQueryInformationProcess)
		goto exit;

	IterateProcesses([](PID pid, const wstring& path) {

			kprintf(TRACE_INFO, "pid=%d, path=%ws", pid, path.c_str());

		});

    //kprintf(TRACE_INFO, "Callback version 0x%hx", ObGetFilterVersion());
  
    //Status = PsSetCreateProcessNotifyRoutineEx (
    //    CreateProcessNotifyRoutine,
    //    FALSE
    //);

    //if (NT_SUCCESS(Status))
    //    ProcessNotifyRoutineSet = TRUE;

exit:
    kprint_st(TRACE_INFO, Status);
    return Status;
}

VOID
PsMonExit()
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

		const PSYSTEM_PROCESS_INFORMATION& operator*() const
		{
			return m_info_ptr;
		}

		iterator& operator++()
		{
			m_offset = m_info_ptr->NextEntryOffset;
			if (m_offset)
				m_info_ptr = (PSYSTEM_PROCESS_INFORMATION)((SIZE_T)m_info_ptr + m_offset);
			else
				m_info_ptr = nullptr;

			return *this;
		}

		bool operator==(const iterator& other) const
		{
			return m_info_ptr == other.m_info_ptr &&
				m_offset == other.m_offset;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}
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

	iterator begin() const 
	{
		return iterator((PSYSTEM_PROCESS_INFORMATION)m_info);
	}

	iterator end() const
	{
		return iterator(nullptr);
	}

private:

	NTSTATUS getProcessList()
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
};

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

NTSTATUS IterateProcesses(function<void(PID, const wstring&)> cb)
{
	NTSTATUS status;
	HANDLE hProcess;
	CLIENT_ID clientId;
	OBJECT_ATTRIBUTES attribs;
	PUNICODE_STRING ImagePath;

	ProcessList proc_list;
	for (const auto& proc : proc_list)
	{
		if (proc->ProcessId == 0)
			continue;

		InitializeObjectAttributes(&attribs, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
		clientId.UniqueProcess = proc->ProcessId;
		clientId.UniqueThread = 0;

		status = ZwOpenProcess(&hProcess, 0x1000/*PROCESS_QUERY_LIMITED_INFORMATION*/, &attribs, &clientId);
		if (!NT_SUCCESS(status))
		{
			kprintf(TRACE_INFO, "Can't open process (pid:%p) failed with code:%08x", proc->ProcessId, status);
			continue;
		}

		status = QueryProcessImagePath(hProcess, &ImagePath);
		if (NT_SUCCESS(status) && ImagePath != nullptr)
		{
			wstring path(ImagePath->Buffer, ImagePath->Length);
			cb(proc->ProcessId, path);
		}
		
		if (ImagePath)
			delete[] (PCHAR)ImagePath;

		ZwClose(hProcess);
	}

	return STATUS_SUCCESS;
}
