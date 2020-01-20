#pragma once

#pragma warning (push, 0)
#include "conslsvc.h"
#pragma warning (pop)

#define LOG_INFO()    getLogStream(LOG_LEVEL_INFO)
#define LOG_WARNING() getLogStream(LOG_LEVEL_WARN)
#define LOG_ERROR()   getLogStream(LOG_LEVEL_ERROR)

namespace drjuke::svc
{
    class AVService final
        : public TConsoleService<FileLogger>
    {
    private:
        LogWriter m_log_writer;

        auto getLogStream(int log_level) -> decltype(m_log_writer.getStreamW(log_level))
        {
            return m_log_writer.getStreamW(log_level);
        }

    public:
        AVService()
            : TConsoleService<FileLogger>(L"AVService")
            , m_log_writer(L"AVService", getLogger())
        {}

        static const auto LOG_LEVEL_INFO  = Logger::LOG_LEVEL_INFORMATION;
        static const auto LOG_LEVEL_WARN  = Logger::LOG_LEVEL_WARNING;
        static const auto LOG_LEVEL_ERROR = Logger::LOG_LEVEL_ERROR;

        DWORD run()        override;
        void  onShutdown() noexcept override;

    };
}