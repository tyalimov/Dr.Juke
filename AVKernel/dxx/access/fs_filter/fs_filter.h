#pragma once

#include "common.h"
#include "access/access_monitor.h"

#define KFF_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\FsFilter"

PFileSystemAccessMonitor FsFilterGetInstancePtr();

NTSTATUS FsFilterInit(PDRIVER_OBJECT DriverObject);

