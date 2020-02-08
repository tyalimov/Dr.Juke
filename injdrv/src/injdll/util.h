#pragma once

#include <ntdll.h>
#include <detours.h>


VOID NTAPI
ProtectDll(HANDLE ModuleHandle);

NTSTATUS NTAPI
QueryKeyValue(const wchar_t* szAbsRegPath,
	const wchar_t* szValueName, NTSTATUS(*onRecord)(PKEY_VALUE_FULL_INFORMATION));

#pragma region ugly_defines

#define GET_PROC_ADDR(FuncName, DllHandle, FuncAddr, VarRoutineName)		\
RtlInitAnsiString(&VarRoutineName, (PSTR)FuncName);							\
LdrGetProcedureAddress(DllHandle, &VarRoutineName, 0, (PVOID*)& FuncAddr);	\
RTL_ASSERT(FuncAddr != nullptr);

#define DETOUR_HOOK(func)							\
Orig_##func = func;									\
DetourAttach((PVOID*)& Orig_##func, Hooked_##func)

#define DETOUR_UNHOOK(func)							\
DetourDetach((PVOID*)& Orig_##func, Hooked_##func) 	

#define DETOUR_HOOK_ON_DLL_LOAD(func, DllHandle, VarRoutineName)				\
RtlInitAnsiString(&VarRoutineName, (PSTR)#func);								\
LdrGetProcedureAddress(DllHandle, &VarRoutineName, 0, (PVOID*)& Orig_##func);	\
RTL_ASSERT(Orig_##func != nullptr);												\
DetourAttach((PVOID*)& Orig_##func, Hooked_##func)

#pragma endregion ugly_defines

