#pragma once

#include <filesystem>
#include <string>

#include <undecorate.h>
#pragma comment (lib, BOOST_UNDECORATE_LIBRARY("boost_log"))

/* 
    Для того, чтобы использовать эту библиотек, нужно:
    1. Добавить в Additional Include Directories: $(SolutionDir)/libs
    2. Добавить в Additional Library Directories: $(ExternalSdk)/boost/stage/lib;

    Пример запуска:

    #include <loglib/logging.h>

    int main()
    {
    	drjuke::log::Initialize();
    	LOG_ERROR("error test");
    	LOG_DEBUG("debug test");
    }
*/

namespace drjuke::log 
{
	void Initialize();
	void LogError(const std::string &message);   
	void LogDebug(const std::string& message);   
}

#define LOG_ERROR(msg) drjuke::log::LogError(msg)

#ifdef _DEBUG
#define LOG_DEBUG(msg) drjuke::log::LogDebug(msg)
#else
#define LOG_DEBUG(msg)
#endif