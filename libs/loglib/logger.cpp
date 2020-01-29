#include "logger.h"

#include <common/aliases.h>
#include <winlib/filesys.h>
#include <winlib/utils.h>
#include <winlib/windows_exception.h>

#include <codecvt>
#include <boost/format.hpp>
#include <sstream>
#include <filesystem>
#include <cassert>

#include <windows.h>
#include <lmcons.h>
#include <shlobj.h>
#include <io.h>
#include <fcntl.h>


#define ERROR_WIDE_STRING L""
#define ERROR_BYTES_STRING ""

const std::wstring kLogStringFormat = L"%02d:%02d:%02d.%03d [%s] %s";
const std::wstring kLogExtension    = L"log"; 

namespace drjuke::loglib
{
    enum LogColor
    {
        kLogTrace = FOREGROUND_BLUE  | FOREGROUND_GREEN     | FOREGROUND_RED       | FOREGROUND_INTENSITY,
        kLogInfo  = FOREGROUND_BLUE  | FOREGROUND_GREEN     | FOREGROUND_INTENSITY,
        kLogWarn  = FOREGROUND_RED   | FOREGROUND_GREEN     | FOREGROUND_INTENSITY,
        kLogFatal = FOREGROUND_RED   | FOREGROUND_BLUE      | FOREGROUND_INTENSITY,
        kLogDebug = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        kLogError = FOREGROUND_RED   | FOREGROUND_INTENSITY,
    };

    namespace
    {
        std::map<LogLevel, std::wstring> g_TypesStr
        {
            { LogLevel::kLogTrace, L"TRACE" },
            { LogLevel::kLogInfo,  L"INFO"  },
            { LogLevel::kLogDebug, L"DEBUG" },
            { LogLevel::kLogWarn,  L"WARN"  },
            { LogLevel::kLogError, L"ERROR" },
            { LogLevel::kLogFatal, L"FATAL" },
        };

        std::map<LogLevel, LogColor> g_TypesColors
        {
            { LogLevel::kLogTrace, LogColor::kLogTrace },
            { LogLevel::kLogInfo,  LogColor::kLogInfo  },
            { LogLevel::kLogDebug, LogColor::kLogDebug },
            { LogLevel::kLogWarn,  LogColor::kLogWarn  },
            { LogLevel::kLogError, LogColor::kLogError },
            { LogLevel::kLogFatal, LogColor::kLogFatal },
        };

        constexpr unsigned int kWaitLogTimeout{ 1000 };
    }

    std::shared_ptr<Logger> LogWriter::m_logger = std::make_shared<Logger>();

    std::wstring ToString(LogLevel type)
    {
        return g_TypesStr[type];
    }
    
    unsigned int GetColor(LogLevel level)
    {
        return g_TypesColors[level];
    }

    void LogWriter::log(const LogLevel &type, const std::wstring &text)
    {
        assert(m_log);

        std::wstring log_text = m_logger_name + L" - " + text;

        if (m_logger)
        {
            m_logger->write(type, text);
        }
    }

    void LogWriter::setMaxLogSize(size_t size)
    {
        m_logger->setMaxLogSize(size);
    }

    void LogWriter::startLog()
    {
        assert(!m_Log && "Log has already started!");
        
        if(!m_logger) 
        {
            m_logger.reset(new Logger());
        }
    }

    void LogWriter::stopLog()
    {
        m_logger.reset();
    }

#define CONSOLE_AGENT_LOG

    Logger::Logger()
        : m_exit(false)
        , m_max_file_size(0)
    {
#ifdef FILE_AGENT_LOG
        createLogFile();
#endif

#ifdef CONSOLE_AGENT_LOG
        createLogConsole();
#endif
        m_log_thread_ptr.reset(new std::thread{ &Logger::runLog, this });
    }

    void Logger::write(const LogLevel &type, const std::wstring &text)
    {
        auto now = winlib::utils::GetCurrentSystemTime();

        std::wstringstream stream;
        
        stream << boost::wformat(kLogStringFormat) 
            % now.wHour 
            % now.wMinute 
            % now.wSecond 
            % now.wMilliseconds 
            % ToString(type) 
            % text 
            << L"\n";
        
        std::wstring format_msg = stream.str();

        std::lock_guard<std::mutex> lock(m_deque_mutex);
        m_log_deque.emplace_back(LogString{ type, format_msg });

        std::lock_guard<std::mutex> non_empty_lock(m_non_empty_queue_mutex);
        m_non_empty_queue.notify_one();
    }

    void Logger::setMaxLogSize(long long size)
    {
        m_max_file_size = size;
    }

    void Logger::writeToConsole(const LogString &text)
    {
        auto& level     = text.first;
        auto& msg       = text.second;	
        auto  color     = GetColor(level);
        auto console    = ::GetStdHandle(STD_OUTPUT_HANDLE);
        
        ::SetConsoleTextAttribute(console, color);
        ::WriteConsoleW
        (
            console, 
            msg.c_str(), 
            msg.size(),
            nullptr,
            nullptr
        );
    }

    Path Logger::getDefaultLogFolder()
    {
        return winlib::filesys::GetDesktopDirectory();
    }

    std::wstring Logger::createLogName() const
    {
        auto time = winlib::utils::GetCurrentSystemTime();
        auto user = winlib::utils::GetCurrentUserName();

        std::wstringstream stream;
        stream << boost::wformat(L"drjuke.%s.%d%02d%02d") 
            % user 
            % time.wYear 
            % time.wMonth 
            % time.wDay;

        return  stream.str();
    }

    bool Logger::emptyDeque()
    {
        std::lock_guard<std::mutex> lock(m_deque_mutex);
        return m_log_deque.empty();
    }

    LogString Logger::popDeque()
    {
        std::lock_guard<std::mutex> lock(m_deque_mutex);
        auto log_string = m_log_deque.front();
        m_log_deque.pop_front();
        return log_string;
    }

    void Logger::runLog()
    {
        while (isLogRunning())
        {
            while (emptyDeque())
            {
                std::unique_lock<std::mutex> lock(m_non_empty_queue_mutex);
                if (m_non_empty_queue.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(kWaitLogTimeout)) == std::cv_status::timeout)
                {
                    if (!isLogRunning())
                    {
                        return;
                    }
                }
            }

            auto log_string = popDeque();

#ifdef CONSOLE_AGENT_LOG
            writeToConsole(log_string);
#endif
#ifdef FILE_AGENT_LOG
            writeToFile(log_string);
#endif
        }
    }

    bool Logger::isLogRunning()
    {
        std::unique_lock<std::mutex> exit_lock(m_exit_mutex);
        return !m_exit;
    }

    void Logger::writeToFile(const LogString &text)
    {
        auto& msg   = text.second;

        int size_needed = ::WideCharToMultiByte
        (
            CP_UTF8, 
            0, 
            msg.data(), 
            static_cast<int>(msg.size()),
            nullptr, 
            0,
            nullptr,
            nullptr
        );
        
        std::string str_to( size_needed, L'\0' );
        
        ::WideCharToMultiByte
        (
            CP_UTF8, 
            0, 
            msg.data(), 
            static_cast<int>(msg.size()), 
            &str_to[0], 
            size_needed,
            nullptr,
            nullptr
        );

        if(m_file.get() != INVALID_HANDLE_VALUE)
        {
            DWORD bytes_written;

            ::WriteFile
            (
                m_file.get(), 
                str_to.data(), 
                str_to.size(), 
                &bytes_written,
                nullptr
            );
        }
    }

    void Logger::createLogFile()
    {
        auto log_dir  = getDefaultLogFolder();
        auto log_mame = createLogName();

        std::wstringstream stream;
        stream << boost::wformat(L"%s\\%s.%s") 
            % log_dir 
            % log_mame 
            % kLogExtension;

        m_log_file_path = stream.str();
        stream.str(std::wstring());

        winlib::filesys::CreateNewFile(m_log_file_path);

        m_file = winlib::UniqueHandle(::CreateFileW
        (
            m_log_file_path.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            nullptr,
            OPEN_ALWAYS,
            0,
            nullptr
        ));

        if (!m_file)
        {
            throw winlib::WindowsException("Can't open log file");
        }
    }

    void Logger::createLogConsole()
    {
        FreeConsole();

        if (!AllocConsole())
        {
            throw winlib::WindowsException("Can't allocate console");
        }

        // set the screen buffer to be big enough to let us scroll text
        const HANDLE h_std_output = ::GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO coninfo;
        ::GetConsoleScreenBufferInfo(h_std_output, &coninfo);

        const SHORT width   = 100;
        const SHORT height  = 1000;

        SMALL_RECT win_info = {0};
        win_info.Right      = width - coninfo.dwSize.X;

        coninfo.dwSize.X    = width;
        coninfo.dwSize.Y    = height;

        const HANDLE h_out  = ::GetStdHandle(STD_OUTPUT_HANDLE);
        assert(hOut);

        ::SetConsoleScreenBufferSize(h_out, coninfo.dwSize);
        ::SetConsoleWindowInfo(h_out, FALSE, &win_info);

        // redirect unbuffered STDOUT to the console	
        intptr_t l_std_handle = reinterpret_cast<intptr_t>(h_out);
        int h_con_handle      = _open_osfhandle(l_std_handle, _O_TEXT);
        const FILE* fp        = _wfdopen( h_con_handle, L"w");
        assert(fp);

        *stdout = *fp;
        setvbuf(stdout, nullptr, _IONBF, 0);

        // redirect unbuffered STDIN to the console
        l_std_handle = reinterpret_cast<intptr_t>(::GetStdHandle(STD_INPUT_HANDLE));
        h_con_handle = _open_osfhandle(l_std_handle, _O_TEXT);
        fp           = _wfdopen(h_con_handle, L"r");
        assert(fp);

        *stdin = *fp;
        setvbuf(stdin, nullptr, _IONBF, 0);

        // redirect unbuffered STDERR to the console
        l_std_handle = reinterpret_cast<intptr_t>(::GetStdHandle(STD_ERROR_HANDLE));
        h_con_handle = _open_osfhandle(l_std_handle, _O_TEXT);
        fp           = _wfdopen(h_con_handle, L"w");
        assert(fp);

        *stderr = *fp;
        setvbuf(stderr, nullptr, _IONBF, 0);

        // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
        std::ios::sync_with_stdio();

        const HWND h_wnd   = ::GetConsoleWindow();
        assert(hWnd);
        const HMENU h_menu = ::GetSystemMenu(h_wnd, FALSE);
        assert(hMenu);

        ::DeleteMenu
        (
            h_menu, 
            SC_CLOSE, 
            MF_BYCOMMAND
        );  //ok actually remove the close button
        
        ::SetWindowPos
        (
            h_wnd,
            nullptr, 
            0, 
            0, 
            0, 
            0, 
            SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME
        );
    }
}
