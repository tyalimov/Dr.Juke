#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h>

#include "preferences.h"
#include "filter.h"
#include "regfilter_cb.h"

PREGFILTER_CALLBACK_CTX gRegFilterCtx = nullptr;


NTSTATUS CreateRegFilterCtx()
{
	NTSTATUS Status = STATUS_SUCCESS;
	gRegFilterCtx = new REGFILTER_CALLBACK_CTX();

	if (gRegFilterCtx == nullptr)
	{
		kprintf(TRACE_LOAD, "Failed to allocate RegFilter context");
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
	{
		RtlInitUnicodeString(&gRegFilterCtx->Altitude, REGFILTER_ALTITUDE);
		gRegFilterCtx->OnRegNtPreCreateKeyEx = OnRegNtPreCreateKeyEx;
		gRegFilterCtx->OnRegNtPostSetValueKey = OnRegNtPostSetValueKey;
		gRegFilterCtx->OnRegFilterInit = OnRegFilterInit;
	}

	return Status;
}

void DeleteRegFilterCtx()
{
	delete gRegFilterCtx;
	gRegFilterCtx = nullptr;
}

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	RegFilterExit(gRegFilterCtx);
	DeleteRegFilterCtx();
	kprintf(TRACE_LOAD, "Driver unloaded");
}

NTSTATUS SysMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	NTSTATUS Status;
	DriverObject->DriverUnload = DriverUnload;
	
	Status = CreateRegFilterCtx();
	if (!NT_SUCCESS(Status))
		goto fail;

	Status = RegFilterInit(DriverObject, gRegFilterCtx);
	if (!NT_SUCCESS(Status))
	{
		DeleteRegFilterCtx();
		goto fail;
	}

	kprintf(TRACE_LOAD, "Driver initialization successfull");
	return STATUS_SUCCESS;

fail:
	kprintf(TRACE_LOAD, "Driver initialization failed");
	kprint_st(TRACE_LOAD, Status);
	return Status;
}
