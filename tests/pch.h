// pch.h
// Header for standard system include files.
//

#pragma once
#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )
#include <vector>
#include <string>
#include <filesystem>

#include <common/constants.h>
#include <common/aliases.h>
#include <common/win_raii.h>