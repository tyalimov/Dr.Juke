#include "log_writer.h"
#include "logger.h"
#include "log_objects.h"

#include <common/aliases.h>

#include <codecvt>
#include <boost/format.hpp>
#include <filesystem>
#include <cassert>

#include <windows.h>
#include <lmcons.h>
#include <shlobj.h>


namespace drjuke::loglib
{
    std::shared_ptr<Logger> LogWriter::m_logger;
    
    // ReSharper disable once CppMemberFunctionMayBeConst
    void LogWriter::log(const LogLevel &type, const std::wstring &text)
    {
        std::wstring log_text = m_logger_name + L" - " + text;

        if (m_logger)
        {
            m_logger->write(type, text);
        }
    }

    void LogWriter::startLog(LogOutput direction)
    {        
        if(!m_logger) 
        {
            m_logger.reset(new Logger(direction));
        }
    }

    void LogWriter::stopLog()
    {
        m_logger.reset();
    }
}
