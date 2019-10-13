#pragma once

#ifdef _DEBUG
#   define CONFIGURATION_NAME "d"
#else
#   define CONFIGURATION_NAME "r"
#endif

#ifdef _M_IX86
#   define PLATFORM_NAME "x86" 
#else
#   define PLATFORM_NAME "x64"
#endif

#ifndef _DLL
#   define LINKAGE_STATUS "s"
#else
#   define LINKAGE_STATUS ""
#endif 

#define UNDECORATE_LIBRARY(library_name) \
    library_name "_" PLATFORM_NAME "_" LINKAGE_STATUS CONFIGURATION_NAME ".lib"