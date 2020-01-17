#pragma once

#include "common.h"

NTSTATUS FsFilterInit(PDRIVER_OBJECT DriverObject);

void FsFilterExit();
