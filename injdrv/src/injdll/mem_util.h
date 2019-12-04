#pragma once
#include "ntdll.h"

namespace ownstl
{
	//void* __cdecl memcpy(void* dst, const void* src, size_t num_bytes);

	//void* __cdecl memset(void* dst, const char val, size_t num_bytes);

	int __cdecl memcmp(const void* left, const void* right, size_t num_bytes);
}
