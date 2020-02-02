#include "updater_workers.h"
#include <iostream>
#include <mutex>
#include <boost/format.hpp>

#include <windows.h>

std::mutex g_ProgressBarMutex;
std::mutex g_ExitMutex;
std::mutex g_CompletionMutex;

bool g_Exit{ false };

std::map<Path, netlib::LoadingProgressPtr> g_Completion;

COORD GetConsoleCursorPosition(HANDLE hConsoleOutput)
{
    CONSOLE_SCREEN_BUFFER_INFO cbsi;

    if (!GetConsoleScreenBufferInfo(hConsoleOutput, &cbsi))
    {
        return { 0, 0 };
    }

    return cbsi.dwCursorPosition;
}

void DrawCompletion()
{
    for (const auto& [key, value] : g_Completion)
    {
        double percentage = (double(value->m_loaded) / double(value->m_total)) * 100.0;

        std::cout << 
            boost::format("%-30s|total= %-10d|loaded= %-9d%%\n") 
                % value->m_filename.filename() 
                % value->m_total
                % percentage;
    }
}

void DrawFinalCompletion()
{
    for (const auto& [key, value] : g_Completion)
    {
        std::cout << 
           boost::format("%-30s|total= %-10d|loaded= %-9d%%\n") 
                % value->m_filename.filename() 
                % value->m_total
                % 100.0000;
    }
}

void DownloaderThread(const std::map<std::string, std::pair<std::string, uint32_t>>& filenames, 
                      const Path& destination)
{
    {
        std::lock_guard<std::mutex> lock(g_CompletionMutex);

        for (const auto& [key, value] : filenames)
        {
            g_Completion[key] = std::make_shared<netlib::LoadingProgress>(key, value.second);
        }
    }

    for (const auto& [key, value] : filenames)
    {
        {
            std::lock_guard<std::mutex> lock(g_ProgressBarMutex);
            g_Completion[key]->m_filename = key;
            g_Completion[key]->m_total    = value.second;
            g_Completion[key]->m_loaded   = 0;
        }
        auto updater  = netlib::Factory::getUpdater(key, destination, g_Completion[key]);
        updater->downloadFile();

    }

    {
        std::lock_guard<std::mutex> lock(g_ExitMutex);
        g_Exit = true;
    }
}

void ProgressBarThread()
{
    auto console_handle       = ::GetStdHandle(STD_OUTPUT_HANDLE);
    auto console_start_coords = GetConsoleCursorPosition(console_handle);

    while (true)
    {
        SetConsoleCursorPosition(console_handle, console_start_coords);

        {
            std::lock_guard<std::mutex> lock(g_ExitMutex);
            if (g_Exit)
            {
                DrawFinalCompletion();
                break;
            }
        }

        {
            std::lock_guard<std::mutex> lock(g_ProgressBarMutex);
            DrawCompletion();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
