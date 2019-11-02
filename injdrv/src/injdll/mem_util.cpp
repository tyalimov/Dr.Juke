#include "mem_util.h"

namespace ownstl
{
	void* __cdecl memcpy(void* dst, const void* src, size_t num_bytes)
	{
		size_t num_4byte = num_bytes / sizeof(DWORD);
		size_t end_4byte = num_4byte * sizeof(DWORD);
		size_t num_1byte = num_bytes - end_4byte;

		for (size_t i = 0; i < num_4byte; i++)
			((DWORD*)dst)[i] = ((DWORD*)src)[i];

		for (size_t i = end_4byte; i < num_bytes; i++)
			((BYTE*)dst)[i] = ((BYTE*)src)[i];

		return dst;
	}

	void* __cdecl memset(void* dst, const char val, size_t num_bytes)
	{
		size_t num_4byte = num_bytes / sizeof(DWORD);
		size_t end_4byte = num_4byte * sizeof(DWORD);
		size_t num_1byte = num_bytes - end_4byte;

		DWORD dw_val = (val << 24) + (val << 16) + (val << 8) + val;
		for (size_t i = 0; i < num_4byte; i++)
			((DWORD*)dst)[i] = dw_val;

		for (size_t i = end_4byte; i < num_bytes; i++)
			((BYTE*)dst)[i] = val;

		return dst;
	}

	int __cdecl memcmp(const void* left, const void* right, size_t num_bytes)
	{
		size_t num_4byte = num_bytes / sizeof(DWORD);
		size_t end_4byte = num_4byte * sizeof(DWORD);
		size_t num_1byte = num_bytes - end_4byte;

		for (size_t i = 0; i < num_4byte; i++)
		{
			if (((DWORD*)left)[i] != ((DWORD*)right)[i])
			{
				if (((DWORD*)left)[i] < ((DWORD*)right)[i])
					return -1;
				else
					return 1;
			}
		}

		for (size_t i = end_4byte; i < num_bytes; i++)
		{
			if (((BYTE*)left)[i] != ((BYTE*)right)[i])
			{
				if (((BYTE*)left)[i] < ((BYTE*)right)[i])
					return -1;
				else
					return 1;
			}
		}

		return 0;
	}
}
