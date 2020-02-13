#pragma once
#include <string>
#include <windows.h>
#include <functional>
#include "trace.h"

namespace drjuke::winlib::utils
{
    std::wstring getCurrentUserName();

    SYSTEMTIME getCurrentSystemTime();
    
    void startProcess(const std::wstring& name, std::wstring argv);
    
    void startService(const std::wstring& name);

    std::wstring getFileKernelPath(const std::wstring& file_path);

    std::wstring getKeyKernelPath(HKEY hBaseKey, const std::wstring& subKey);

    void injDrvStartTrace(injdrv::TraceCallback onMessage);

    void injDrvStopTrace();
}
