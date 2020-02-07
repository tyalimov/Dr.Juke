#pragma once
#include "common.h"
#include "access/access_monitor.h"

#define KRF_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\RegFilter"
#define REGFILTER_ALTITUDE L"380010"

void RegFilterExit();

PRegistryAccessMonitor RegFilterGetInstancePtr();

NTSTATUS RegFilterInit(PDRIVER_OBJECT DriverObject);
