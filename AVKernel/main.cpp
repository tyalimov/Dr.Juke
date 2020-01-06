#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>

#include "rcn.h"
#include "preferences.h"
#include "filter.h"

RCN_THREAD_CTX gRCNThread;
BOOLEAN bRCNInit = FALSE;

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	if (bRCNInit)
		RCNStopThread(&gRCNThread);

	kprintf(TRACE_LOAD, "Driver unloaded");
}


NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS Status;
	DrvObject->DriverUnload = DriverUnload;
	
	Status = PreferencesReset(KEY_PROCFILTER);
	if (!NT_SUCCESS(Status))
		goto fail;

	gRCNThread.Trackers.push_back({ KEY_PROCFILTER, OnProcFilterKeyChange });
	Status = RCNStartThread(&gRCNThread);
	if (!NT_SUCCESS(Status))
		goto fail;

	bRCNInit = TRUE;
	kprintf(TRACE_LOAD, "Driver initialization successfull");
	return STATUS_SUCCESS;

fail:
	kprintf(TRACE_LOAD, "Driver initialization failed");
	kprint_st(TRACE_LOAD, Status);
	return Status;
}
