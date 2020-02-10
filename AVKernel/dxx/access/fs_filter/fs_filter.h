#pragma once

#include "common.h"
#include "access/access_monitor.h"

#define KFF_BASE_KEY LR"(\REGISTRY\MACHINE\SOFTWARE\Dr.Juke\FilesystemFilterRules)"

PFileSystemAccessMonitor FsFilterGetInstancePtr();

NTSTATUS FsFilterInit(PDRIVER_OBJECT DriverObject);

