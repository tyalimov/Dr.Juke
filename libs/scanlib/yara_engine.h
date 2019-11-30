#pragma once

#include <undecorate.h>

#pragma warning( push )

#   pragma warning( disable : 4081 )
#   include <yaracpp.h>
#   pragma comment( lib, UNDECORATE_LIBRARY("yaracpp") )
#   pragma comment( lib, UNDECORATE_LIBRARY("libyara") )
#   pragma comment( lib, "ws2_32.lib" )
#   pragma comment( lib, "crypt32.lib" )

#pragma warning( pop )