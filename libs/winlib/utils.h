#pragma once
#include <string>
#include <minwinbase.h>
namespace drjuke::winlib::utils
{
    std::wstring GetCurrentUserName();
    SYSTEMTIME GetCurrentSystemTime();
}
