#pragma once

#include <Windows.h>

void HandleBorneEmplace(HANDLE parent, HANDLE child);

HANDLE HandleBorneGetParent(HANDLE child);

void HandleBorneEraseReqursive(HANDLE node);
