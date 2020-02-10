#include "handle_borne.h"
#include <map>

using namespace std;

using HandleBorne = map<HANDLE, HANDLE>;
HandleBorne g_child_parent_map;
HandleBorne g_parent_child_map;

void HandleBorneEmplace(HANDLE parent, HANDLE child)
{
	g_parent_child_map.emplace(parent, child);
	g_child_parent_map.emplace(child, parent);
}

HANDLE HandleBorneGetParent(HANDLE child)
{
	auto it = g_child_parent_map.find(child);
	if (it != g_child_parent_map.end())
		return it->second;

	return NULL;
}

HANDLE HandleBorneGetChild(HANDLE parent)
{
	auto it = g_parent_child_map.find(parent);
	if (it != g_parent_child_map.end())
		return it->second;

	return NULL;
}

void HandleBorneEraseReqursive(HANDLE node,
	std::function<void(HANDLE)> onDelete)
{
	HandleBorneEraseParentsReq(node, onDelete);
	HandleBorneEraseChildrenReq(node, onDelete);
}

void HandleBorneEraseChildrenReq(HANDLE node,
	std::function<void(HANDLE)> onDelete)
{
	auto it_parent = g_child_parent_map.find(node);
	if (it_parent != g_child_parent_map.end())
		HandleBorneEraseChildrenReq(it_parent->second, onDelete);

	onDelete(node);
	g_child_parent_map.erase(node);
}

void HandleBorneEraseParentsReq(HANDLE node,
	std::function<void(HANDLE)> onDelete)
{
	auto it_parent = g_parent_child_map.find(node);
	if (it_parent != g_parent_child_map.end())
		HandleBorneEraseParentsReq(it_parent->second, onDelete);

	onDelete(node);
	g_parent_child_map.erase(node);
}
