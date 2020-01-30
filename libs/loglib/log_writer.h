#pragma once

#include "string_conv.h"
#include "log_types.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <memory>
#include <winlib/raii.h>

namespace drjuke::loglib
{
    class Logger;

    class LogWriter final
        : private boost::noncopyable
    {
    public:
        explicit LogWriter(const std::wstring& log_name)
            : m_logger_name(log_name)
        {}

        explicit LogWriter(const std::string& log_name)
            : m_logger_name(ToWstring(log_name))
        {}

        void log(const LogLevel& type, const std::wstring& text);
        
        static void startLog(LogOutput direction);
        static void stopLog();

    private:
        static std::shared_ptr<Logger> m_logger;
        std::wstring                   m_logger_name;
    };
}