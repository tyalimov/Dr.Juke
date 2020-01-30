#pragma once
#include <string>
#include <windows.h>

namespace drjuke::winlib::utils
{
    std::wstring GetCurrentUserName();
    SYSTEMTIME GetCurrentSystemTime();
}
