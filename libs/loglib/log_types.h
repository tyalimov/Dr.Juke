#pragma once

#include <map>
#include <windows.h>
#include <string>

#define  WAIT_LOG_TIMEOUT 1000

namespace drjuke::loglib
{
    enum class LogLevel
    {
        kLogTrace,
        kLogDebug,
        kLogInfo,
        kLogWarn,
        kLogError,
        kLogFatal
    };

    enum class LogOutput
    {
        kRemoteConsole,
        kFile,
        kDefaultConsole
    };

    enum LogColor
    {
        kLogTrace = FOREGROUND_BLUE  | FOREGROUND_GREEN     | FOREGROUND_RED       | FOREGROUND_INTENSITY,
        kLogInfo  = FOREGROUND_BLUE  | FOREGROUND_GREEN     | FOREGROUND_INTENSITY,
        kLogWarn  = FOREGROUND_RED   | FOREGROUND_GREEN     | FOREGROUND_INTENSITY,
        kLogFatal = FOREGROUND_RED   | FOREGROUND_BLUE      | FOREGROUND_INTENSITY,
        kLogDebug = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        kLogError = FOREGROUND_RED   | FOREGROUND_INTENSITY,
    };
}
