#include "logging.h"

#pragma warning (push, 0)
#   include <boost/log/trivial.hpp>
#   include <boost/log/utility/setup.hpp>
#pragma warning (pop)

using namespace boost::log;

namespace drjuke::log
{
	void Initialize()
	{
		register_simple_formatter_factory<trivial::severity_level, char>("Severity");

		// Output message to console
		add_console_log
		(
			std::cout,
			keywords::format     = "[%TimeStamp%][%Severity%]:  %Message%",
			keywords::auto_flush = true
		);

		add_common_attributes();
	}

	void LogError(const std::string& message)
	{
		BOOST_LOG_TRIVIAL(error) << message;
	}
	void LogDebug(const std::string& message)
	{
		BOOST_LOG_TRIVIAL(debug) << message;
	}
};