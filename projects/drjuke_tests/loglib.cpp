#include "pch.h"

#include <loglib/loglib.h>

using namespace av::log;

TEST(default_test, loglib)
{
	LogParameters params("", "[test]");

	InitializeLog(LogOutput::kConsole, params);

	BOOST_LOG_TRIVIAL(trace) << "Hello World\n";
}