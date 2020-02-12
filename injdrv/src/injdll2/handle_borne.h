#pragma once

#include <Windows.h>
#include <functional>

#define __parent_child__(p, c)

void HandleBorneEmplace(HANDLE parent, HANDLE child);

HANDLE HandleBorneGetParent(HANDLE child);

HANDLE HandleBorneGetChild(HANDLE parent);

void HandleBorneEraseReqursive(HANDLE node);

void HandleBorneEraseChildrenReq(HANDLE node);

void HandleBorneEraseParentsReq(HANDLE node);
