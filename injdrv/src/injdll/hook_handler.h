#pragma once

#define NTDLL_NO_INLINE_INIT_STRING
#include <ntdll.h>
#include "api_call.h"

VOID NTAPI
HookHandlerLower(ApiCall* call);
