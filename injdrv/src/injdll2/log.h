#pragma once

#include <stdio.h>

//#define DEBUG_ON

#ifdef DEBUG_ON
#define dbg(fmt, ...) printf("%s: " ## fmt ## "\n", __FUNCTION__, __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif
