#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include "preferences.h"
#include "reg_filter.h"
#include "fs_filter.h"
#include "ps_monitor.h"

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	RegFilterExit();
	PsMonExit();

	kprintf(TRACE_LOAD, "Driver unloaded");
}

NTSTATUS SysMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS Status;
	DriverObject->DriverUnload = DriverUnload;

	if (false)
		goto fail;

	Status = PsMonInit();
	if (!NT_SUCCESS(Status))
		goto fail;

	Status = RegFilterInit(DriverObject);
	if (!NT_SUCCESS(Status))
	{
		PsMonExit();
		goto fail;
	}

	//kprintf(TRACE_LOAD, "FsFilter init: 0x%08X", Status);

	kprintf(TRACE_LOAD, "Driver initialization successfull");
	return STATUS_SUCCESS;

fail:
	kprintf(TRACE_LOAD, "Driver initialization failed");
	kprint_st(TRACE_LOAD, Status);
	return STATUS_SUCCESS;
}
