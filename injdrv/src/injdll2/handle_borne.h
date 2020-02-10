#pragma once

#include <Windows.h>
#include <functional>

#define __parent_child__(p, c)

void HandleBorneEmplace(HANDLE parent, HANDLE child);

HANDLE HandleBorneGetParent(HANDLE child);

HANDLE HandleBorneGetChild(HANDLE parent);

void HandleBorneEraseReqursive(HANDLE node,
	std::function<void(HANDLE)> onDelete);

void HandleBorneEraseChildrenReq(HANDLE node,
	std::function<void(HANDLE)> onDelete);

void HandleBorneEraseParentsReq(HANDLE node,
	std::function<void(HANDLE)> onDelete);
