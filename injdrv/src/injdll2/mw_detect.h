#pragma once


#include "api_call.h"
#include <Windows.h>

#define __start_func__(m)
#define __middle_func__(m, n)
#define __trigger_func__(m)

bool mwDetectProcessHollowing2(HANDLE hProcess);

bool mwDetectProcessHollowing3(HANDLE hProcess);

bool mwDetectProcessHollowing4(HANDLE hThread);

bool mwDetectProcessHollowing(HANDLE hThread, ApiCall* call);

bool mwDetectSimpleProcessInjection(HANDLE hProcess, ApiCall* call);

bool mwDetectThreadHijacking2(HANDLE hThread);

bool mwDetectThreadHijacking(HANDLE hThread, ApiCall* call);

bool mwDetectApcInjection(HANDLE ApcRoutine, ApiCall* call);

bool mwDetectEarlyBird2(HANDLE hProcess);

bool mwDetectEarlyBird(HANDLE hThread, PVOID ApcRoutine, ApiCall* call);
