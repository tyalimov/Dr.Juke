#pragma once

#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>
#include <detours.h>


VOID NTAPI
ProtectDll(HANDLE ModuleHandle);

NTSTATUS NTAPI
QueryKeyValue(const wchar_t* szAbsRegPath,
	const wchar_t* szValueName, NTSTATUS(*onRecord)(PKEY_VALUE_FULL_INFORMATION));

#define GET_PROC_ADDR(FuncName, DllHandle, FuncAddr, VarRoutineName)		\
RtlInitAnsiString(&VarRoutineName, (PSTR)FuncName);							\
LdrGetProcedureAddress(DllHandle, &VarRoutineName, 0, (PVOID*)& FuncAddr);	\
RTL_ASSERT(FuncAddr != nullptr);
