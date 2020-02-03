#pragma once
#include <string>
#include <windows.h>

namespace drjuke::winlib::utils
{
    std::wstring getCurrentUserName();
    SYSTEMTIME getCurrentSystemTime();
    void startProcess(const std::wstring& name, std::wstring argv);
    void startService(const std::wstring& name);
}
