#pragma once

#include "common.h"

VOID NetFilterExit();

NTSTATUS NetFilterInit(PDRIVER_OBJECT DriverObject);
