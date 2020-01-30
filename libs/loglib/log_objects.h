#pragma once

#include "log_types.h"

namespace drjuke::loglib
{
    static std::map<LogLevel, std::wstring> g_TypesStr
    {
        { LogLevel::kLogTrace, L"TRACE" },
        { LogLevel::kLogInfo,  L"INFO"  },
        { LogLevel::kLogDebug, L"DEBUG" },
        { LogLevel::kLogWarn,  L"WARN"  },
        { LogLevel::kLogError, L"ERROR" },
        { LogLevel::kLogFatal, L"FATAL" },
    };

    static std::map<LogLevel, LogColor> g_TypesColors
    {
        { LogLevel::kLogTrace, LogColor::kLogTrace },
        { LogLevel::kLogInfo,  LogColor::kLogInfo  },
        { LogLevel::kLogDebug, LogColor::kLogDebug },
        { LogLevel::kLogWarn,  LogColor::kLogWarn  },
        { LogLevel::kLogError, LogColor::kLogError },
        { LogLevel::kLogFatal, LogColor::kLogFatal },
    };

    static std::wstring ToString(LogLevel type)
    {
        return g_TypesStr[type];
    }
    
    static unsigned int GetColor(LogLevel level)
    {
        return g_TypesColors[level];
    }
}