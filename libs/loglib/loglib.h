#pragma once

#include <filesystem>
#include <exception>

namespace av::log
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

		explicit LogParameters(const fs::path& file_path, const std::string& component_name)
			: m_file_path(file_path)
			, m_component_name(component_name)
		{}

		LogParameters() = delete;
	};

    class UninitializedLogException final : public std::exception
    {
		//virtual const char* what() override
        //{
		//	return "Logging library wasn't initialized properly, "
		//		"call InitializeLog before using it";
        //}
    };

}