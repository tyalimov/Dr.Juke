#include "logger.h"
#include "log_objects.h"

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
#include <iostream>

#define ERROR_WIDE_STRING L""
#define ERROR_BYTES_STRING ""

const std::wstring kLogStringFormat = L"%02d:%02d:%02d.%03d [%s] %s";
const std::wstring kLogExtension    = L".log"; 

namespace drjuke::loglib
{

    Logger::Logger(LogOutput direction)
        : m_direction(direction)
        , m_exit(false)
        , m_max_file_size(0)
    {

        switch (m_direction)
        {
            case LogOutput::kFile:           createLogFile();                     break;
            case LogOutput::kRemoteConsole:  createLogConsole();                  break;
            case LogOutput::kDefaultConsole: /*do nothing*/                       break;
            default:                         createLogFile();                     break;
        }

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

    void Logger::writeToConsole(const LogString &text)
    {
        auto& level     = text.first;
        auto& msg       = text.second;	
        auto  color     = GetColor(level);
        auto  console   = ::GetStdHandle(STD_OUTPUT_HANDLE);
        
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
        return winlib::filesys::getDesktopDirectory();
    }

    std::wstring Logger::createLogName() const
    {
        auto time = winlib::utils::GetCurrentSystemTime();
       
        std::wstringstream stream;
        stream << boost::wformat(L"drjuke_log_%d.%02d.%02d.%s") 
            % time.wDay
            % time.wMonth 
            % time.wYear 
            % kLogExtension;

        return stream.str();
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
                if (m_non_empty_queue.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(WAIT_LOG_TIMEOUT)) == std::cv_status::timeout)
                {
                    if (!isLogRunning())
                    {
                        return;
                    }
                }
            }

            auto log_string = popDeque();

            switch (m_direction)
            {
                case LogOutput::kFile:           writeToFile(log_string);    break;
                case LogOutput::kRemoteConsole:  writeToConsole(log_string); break;
                case LogOutput::kDefaultConsole: writeToDefaultConsole(log_string); break;
            }
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

    void Logger::writeToDefaultConsole(const LogString &text)
    {
        std::wcout << text.second << std::endl;
    }

    void Logger::createLogFile()
    {
        auto log_dir  = getDefaultLogFolder();
        auto log_mame = createLogName();

        m_log_file_path = log_dir / log_mame;

        winlib::filesys::deleteFile(m_log_file_path);
        winlib::filesys::createFile(m_log_file_path);

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
