#include "handle_borne.h"
#include <map>
#include <mutex>

using namespace std;

using HandleBorne = map<HANDLE, HANDLE>;
HandleBorne g_child_parent_map;
HandleBorne g_parent_child_map;
mutex g_hb_lock;

void HandleBorneEmplace(HANDLE parent, HANDLE child)
{
	lock_guard<mutex> guard(g_hb_lock);

	g_parent_child_map.emplace(parent, child);
	g_child_parent_map.emplace(child, parent);
}

HANDLE HandleBorneGetParent(HANDLE child)
{
	lock_guard<mutex> guard(g_hb_lock);

	auto it = g_child_parent_map.find(child);
	if (it != g_child_parent_map.end())
		return it->second;

	return NULL;
}

HANDLE HandleBorneGetChild(HANDLE parent)
{
	lock_guard<mutex> guard(g_hb_lock);

	auto it = g_parent_child_map.find(parent);
	if (it != g_parent_child_map.end())
		return it->second;

	return NULL;
}

void HandleBorneEraseReqursive(HANDLE node,
	std::function<void(HANDLE)> onDelete)
{
	lock_guard<mutex> guard(g_hb_lock);
	HandleBorneEraseParentsReq(node, onDelete);
	HandleBorneEraseChildrenReq(node, onDelete);
}

void HandleBorneEraseParentsReq(HANDLE node,
	std::function<void(HANDLE)> onDelete)
{
	auto it_parent = g_child_parent_map.find(node);
	if (it_parent != g_child_parent_map.end())
		HandleBorneEraseParentsReq(it_parent->second, onDelete);

	onDelete(node);
	g_child_parent_map.erase(node);
}

void HandleBorneEraseChildrenReq(HANDLE node,
	std::function<void(HANDLE)> onDelete)
{
	auto it_parent = g_parent_child_map.find(node);
	if (it_parent != g_parent_child_map.end())
		HandleBorneEraseChildrenReq(it_parent->second, onDelete);

	onDelete(node);
	g_parent_child_map.erase(node);
}
