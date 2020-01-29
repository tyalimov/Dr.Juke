#pragma once

#include <boost/noncopyable.hpp>
#include <string>
#include <memory>
#include "string_conv.h"
#include <deque>
#include <mutex>
#include <common/aliases.h>
#include <winlib/raii.h>

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
        
        static void setMaxLogSize(size_t size);
        static void startLog();
        static void stopLog();

    private:
        static std::shared_ptr<Logger> m_logger;
        std::wstring                   m_logger_name;
    };

    using LogString = std::pair<LogLevel, std::wstring>;

    class Logger final
        : private boost::noncopyable
    {
    public:
        Logger();
        
        void write(const LogLevel& type, const std::wstring& text);		
        void setMaxLogSize(__int64 size);

    protected:
        void writeToConsole(const LogString& text);
        void writeToFile(const LogString& text);
        void createLogFile();
        void createLogConsole();

        [[nodiscard]] static Path getDefaultLogFolder();
        [[nodiscard]] std::wstring createLogName() const;

        bool emptyDeque();
        LogString popDeque();

        void runLog();
        bool isLogRunning();
    private:

        std::deque<LogString>        m_log_deque;
        std::mutex                   m_deque_mutex;
        std::shared_ptr<std::thread> m_log_thread_ptr;

        bool                         m_exit;
        std::condition_variable      m_non_empty_queue;
        std::mutex                   m_exit_mutex;
        std::mutex                   m_non_empty_queue_mutex;

        winlib::UniqueHandle         m_file;

        std::wstring                 m_log_file_path;
        size_t                       m_max_file_size;
    };
}
