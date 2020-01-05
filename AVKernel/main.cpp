#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>

#include "common.h"
#include "rcn.h"


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	RCNExit();
	kprintf(TRACE_LOAD, "Driver unloaded");
}


NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	DrvObject->DriverUnload = DriverUnload;
	
	NTSTATUS Status = RCNInit();
	if (!NT_SUCCESS(Status))
		goto fail;

	kprintf(TRACE_LOAD, "Driver initialization successfull");
	return STATUS_SUCCESS;

fail:
	kprintf(TRACE_LOAD, "Driver initialization failed");
	kprint_st(TRACE_LOAD, Status);
	return Status;
}
