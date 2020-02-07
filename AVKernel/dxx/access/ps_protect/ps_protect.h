#pragma once

#include "common.h"
#include "access/access_monitor.h"

#define KPF_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\PsMonitor"

PProcessAccessMonitor PsProtectGetInstancePtr();

NTSTATUS PsProtectInit();

VOID PsProtectExit();

//NTSTATUS PsProtectExit(PDRIVER_OBJECT DriverObject);
