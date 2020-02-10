#include "handle_borne.h"
#include <map>

using namespace std;

using HandleBorne = map<HANDLE, HANDLE>;
HandleBorne g_handle_borne_map;

void HandleBorneEmplace(HANDLE parent, HANDLE child)
{
	g_handle_borne_map.emplace(child, parent);
}

HANDLE HandleBorneGetParent(HANDLE child)
{
	auto it = g_handle_borne_map.find(child);
	if (it != g_handle_borne_map.end())
		return it->second;

	return NULL;
}

void HandleBorneEraseReqursive(HANDLE node)
{
	auto it_parent = g_handle_borne_map.find(node);
	if (it_parent != g_handle_borne_map.end())
		HandleBorneEraseReqursive(it_parent->second);

	g_handle_borne_map.erase(it_parent);
}
