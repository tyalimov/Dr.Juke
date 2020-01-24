#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include "preferences.h"
#include "reg_filter.h"
#include "fs_filter.h"
#include "ps_monitor.h"

ZwQuerySystemInformationRoutine ZwQuerySystemInformation = nullptr;
ZwQueryInformationProcessRoutine ZwQueryInformationProcess = nullptr;

BOOLEAN DynamicLoadRoutines()
{
	UNICODE_STRING routine;

	RtlInitUnicodeString(&routine, L"ZwQuerySystemInformation");
	ZwQuerySystemInformation = (ZwQuerySystemInformationRoutine)MmGetSystemRoutineAddress(&routine);
		
	RtlInitUnicodeString(&routine, L"ZwQueryInformationProcess");
	ZwQueryInformationProcess = (ZwQueryInformationProcessRoutine)MmGetSystemRoutineAddress(&routine);

	return ZwQuerySystemInformation != nullptr
		&& ZwQueryInformationProcess != nullptr;
}

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	PsMonExit();
	RegFilterExit();

	kprintf(TRACE_LOAD, "Driver unloaded");
}

NTSTATUS SysMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS Status;
	DriverObject->DriverUnload = DriverUnload;

	if (false)
		goto fail;

	if (!DynamicLoadRoutines())
	{
		kprintf(TRACE_ERROR, "Failed to load required system routines!");
		goto fail;
	}

	Status = RegFilterInit(DriverObject);
	if (!NT_SUCCESS(Status))
		goto fail;

	Status = PsMonInit();
	if (!NT_SUCCESS(Status))
	{
		RegFilterExit();
		goto fail;
	}

	kprintf(TRACE_LOAD, "Driver initialization successfull");
	return STATUS_SUCCESS;

fail:
	kprintf(TRACE_LOAD, "Driver initialization failed");
	return STATUS_SUCCESS;
}
