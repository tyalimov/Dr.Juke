#pragma once

#include <ntdll.h>
#include "api_call.h"

VOID NTAPI
HookHandlerLower(ApiCall* call);
