#pragma once

#include <Windows.h>
#include "api_call.h"

void RefHandlesEmplace(CallId id, HANDLE handle);

bool RefHandlesIsReferenced(CallId id, HANDLE handle);

void RefHandlesErase(CallId id, HANDLE handle);
