#pragma once


#pragma warning (push, 0)
#   include <filesystem>
#   include <iostream>
#   include <boost/log/trivial.hpp>
#   include <boost/log/utility/setup.hpp>
#pragma warning (pop)

namespace drjuke::log 
{
    namespace fs = std::filesystem;

	enum class LogOutput
	{
		kConsole,
		kFile,
		kBoth
	};

	enum class LogLibraryStatus
	{
		kInitialized,
	    kUninitialized
    };

	struct LogParameters
	{
		fs::path    m_file_path;
		std::string m_component_name;

		explicit LogParameters(const fs::path    &file_path, 
			                   const std::string &component_name)
			: m_file_path(file_path)
			, m_component_name(component_name)
		{}

		LogParameters() = delete;
	};

	void InitializeLog(LogOutput output, const LogParameters& params);
}