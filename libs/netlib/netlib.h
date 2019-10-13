#pragma once

#include <undecorate.h>

#pragma region Includes
#   include <curl.h>
#pragma endregion 

#pragma warning( push )                                   
#   pragma warning(disable : 4081)                         
#   pragma comment("lib", UNDECORATE_LIBRARY("libcurl"))
#pragma warning( pop ) 