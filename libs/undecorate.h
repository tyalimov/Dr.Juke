#pragma once

#ifdef _DEBUG
#   define CONFIGURATION_NAME    "d"
#else
#   define CONFIGURATION_NAME    "r"
#endif

#ifdef _M_IX86
#   define PLATFORM_NAME         "x86" 
#   define BOOST_PLATFORM_NAME   "x32" 
#else
#   define PLATFORM_NAME         "x64"
#   define BOOST_PLATFORM_NAME   "x64" 
#endif

#ifndef _DLL
#   define LINKAGE_STATUS        "s"
#   define BOOST_LINKAGE_STATUS  "gd"
#else
#   define LINKAGE_STATUS        ""
#   define BOOST_LINKAGE_STATUS  ""
#endif 

#define UNDECORATE_LIBRARY(library_name) \
    library_name "_" PLATFORM_NAME "_" LINKAGE_STATUS CONFIGURATION_NAME ".lib"

#define BOOST_UNDECORATE_LIBRARY(library_name) \
    "boost_" library_name "-vc142" "-mt" "-" BOOST_LINKAGE_STATUS "-" BOOST_PLATFORM_NAME "-1_71" ".lib"

#define LINK_LIBRARY(name) \
    __pragma( comment ( lib, UNDECORATE_LIBRARY(name)) )

#define BOOST_LINK_LIBRARY(name) \
    __pragma( comment ( lib, BOOST_UNDECORATE_LIBRARY(name)) )

#define LINK_YARA                                              \
    __pragma( comment( lib, UNDECORATE_LIBRARY("yaracpp")) )   \
    __pragma( comment( lib, UNDECORATE_LIBRARY("libyara")) )   \
    __pragma( comment( lib, "ws2_32.lib" ))                    \
    __pragma( comment( lib, "crypt32.lib" ))