#pragma once

#include "common.h"

#define KNF_BASE_KEY LR"(\REGISTRY\MACHINE\SOFTWARE\Dr.Juke\FirewallRules)"

VOID NetFilterExit();

NTSTATUS NetFilterInit(PDRIVER_OBJECT DriverObject);
