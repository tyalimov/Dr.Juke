#include <filesystem>
#include "loglib.h"

namespace av::log
{
	namespace 
    {
		LogLibraryStatus  g_InitStatus = LogLibraryStatus::kUninitialized;
		const std::string g_LogFormatString("[%TimeStamp%][%Severity%]:  %Message%");

	    void InitializeConsoleLog()
	    {
			boost::log::register_simple_formatter_factory
	        <boost::log::trivial::severity_level, char>("Severity");

			// Output message to console
			boost::log::add_console_log
	        (
				std::cout,
				boost::log::keywords::format     = g_LogFormatString,
				boost::log::keywords::auto_flush = true
			);
	    }
	    void InitializeFileLog(const LogParameters &params)
	    {
			boost::log::add_file_log
	        (
				boost::log::keywords::file_name           = params.m_file_path.generic_wstring() + L"_%3N.log" ,
				boost::log::keywords::rotation_size       = 1 * 1024 * 1024,
				boost::log::keywords::max_size            = 20 * 1024 * 1024,
				boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
				boost::log::keywords::format              = g_LogFormatString,
				boost::log::keywords::auto_flush          = true
			);
	    }
        void InitializeBothLog(const LogParameters &params)
	    {
			InitializeConsoleLog();
			InitializeFileLog(params);
	    }
        void FinalizeLog()
	    {
			boost::log::add_common_attributes();
			
#ifndef _DEBUG
			boost::log::core::get()->set_filter
	        (
				boost::log::trivial::severity >= boost::log::trivial::info
			);
#endif
	    }
	}

	void InitializeLog(LogOutput output, const LogParameters &params)
	{
        if (g_InitStatus == LogLibraryStatus::kInitialized)
        {
			return;
        }

        switch (output)
        {
		case LogOutput::kFile:    InitializeFileLog(params); break;
		case LogOutput::kConsole: InitializeConsoleLog();    break;
		case LogOutput::kBoth:    InitializeBothLog(params); break;
		}
		
		FinalizeLog();
	}
};