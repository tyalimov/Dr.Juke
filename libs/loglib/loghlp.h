#pragma once

#include "log_stream.h"
#include "logger.h"

#include <sstream>
#include <tchar.h>
#include <memory>

#define CLASS_LOGGER_DECLARATION() \
    static const std::shared_ptr<drjuke::loglib::LogWriter> m_logger_ins;

#define CLASS_LOGGER_PLACEMENT_AND_INIT(class_name) \
    const std::shared_ptr<drjuke::loglib::LogWriter> class_name::m_logger_ins = \
    std::make_shared<drjuke::loglib::LogWriter>(_T(#class_name));


#define DECLARE_CLASS_LOGGER() \
    CLASS_LOGGER_DECLARATION()

#define IMPLEMENT_CLASS_LOGGER(class_name) \
    CLASS_LOGGER_PLACEMENT_AND_INIT(class_name)


#	define LOG_WITH_LEVEL(level, msg)                 \
        do                                            \
        {	                                          \
            if (!(m_logger_ins))                      \
            {                                         \
                break;                                \
            }	                                      \
                                                      \
            drjuke::loglib::LogStream ss;             \
            ss << msg;                                \
            (m_logger_ins)->log(level, ss.str());     \
        }                                             \
        while (false); 

#   define LOG_TRACE(msg)  LOG_WITH_LEVEL(drjuke::loglib::LogLevel::kLogTrace, msg)
#   define LOG_DEBUG(msg)  LOG_WITH_LEVEL(drjuke::loglib::LogLevel::kLogDebug, msg)
#   define LOG_INFO (msg)  LOG_WITH_LEVEL(drjuke::loglib::LogLevel::kLogInfo,  msg)
#   define LOG_WARN (msg)  LOG_WITH_LEVEL(drjuke::loglib::LogLevel::kLogWarn,  msg)
#   define LOG_ERROR(msg)  LOG_WITH_LEVEL(drjuke::loglib::LogLevel::kLogError, msg)
#   define LOG_FATAL(msg)  LOG_WITH_LEVEL(drjuke::loglib::LogLevel::kLogFatal, msg)

#   define LOGHLP_TRY_CATCH_BLOCK(statement) try { statement; } catch (...) {}

#   define SAFE_LOG_TRACE(msg)  LOGHLP_TRY_CATCH_BLOCK(LOG_TRACE(msg))
#   define SAFE_LOG_DEBUG(msg)  LOGHLP_TRY_CATCH_BLOCK(LOG_DEBUG(msg))
#   define SAFE_LOG_INFO (msg)  LOGHLP_TRY_CATCH_BLOCK(LOG_INFO (msg))
#   define SAFE_LOG_WARN (msg)  LOGHLP_TRY_CATCH_BLOCK(LOG_WARN (msg))
#   define SAFE_LOG_ERROR(msg)  LOGHLP_TRY_CATCH_BLOCK(LOG_ERROR(msg))
#   define SAFE_LOG_FATAL(msg)  LOGHLP_TRY_CATCH_BLOCK(LOG_FATAL(msg))