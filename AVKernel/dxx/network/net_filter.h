#pragma once

#include "common.h"

#define KNF_BASE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Dr.Juke\\AVSecGeneric\\NetFilter"

VOID NetFilterExit();

NTSTATUS NetFilterInit(PDRIVER_OBJECT DriverObject);
