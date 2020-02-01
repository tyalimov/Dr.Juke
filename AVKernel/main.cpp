#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include "preferences.h"
#include "reg_filter.h"
#include "fs_filter.h"
#include "ps_monitor.h"
#include "ps_protect.h"
#include "util.h"

ZwQuerySystemInformationRoutine ZwQuerySystemInformation = nullptr;
ZwQueryInformationProcessRoutine ZwQueryInformationProcess = nullptr;

bool DynamicLoadRoutines()
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
	
	//
	// Process monitor must exit first
	// because it uses filter context pointers
	//
	// If we call XXXFilterExit before it
	// there's may be a null-deref during driver unload
	//

	PsMonExit();

	RegFilterExit();

	// Callback is called instead
	// FsFilterExit();

	PsProtectExit();

	kprintf(TRACE_INFO, "Driver unloaded");
}

// TODO: 
// 1) Set filter unloadable
// 2) properly handle IRP_MJ_SHUTDOWN event (register IOREgisterShutdownEvent)
// 3) setup tracing options via DEFINE and Cond compile DRIVER_SUPPORTS_UNLOAD

NTSTATUS SysMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS Status;
	DriverObject->DriverUnload = DriverUnload;

	if (!IsWin8OrGreater())
	{
		kprintf(TRACE_WARN, "Target OS is not Windows 8 or greater. "
			"Named pipe protection is not supported");
	}

	if (!DynamicLoadRoutines())
	{
		kprintf(TRACE_ERROR, "Failed to load required system routines!");
		goto fail;
	}

	Status = PsMonInit();
	if (!NT_SUCCESS(Status))
		goto fail;

	Status = RegFilterInit(DriverObject);
	if (!NT_SUCCESS(Status))
	{
		PsMonExit();
		goto fail;
	}

	Status = FsFilterInit(DriverObject);
	if (!NT_SUCCESS(Status))
	{
		PsMonExit();
		RegFilterExit();
		goto fail;
	}

	Status = PsProtectInit();
	if (!NT_SUCCESS(Status))
	{
		PsMonExit();
		RegFilterExit();
		goto fail;
	}


	kprintf(TRACE_INFO, "Driver initialization successfull");
	return STATUS_SUCCESS;

fail:
	kprintf(TRACE_INFO, "Driver initialization failed");
	return STATUS_SUCCESS;
}
