#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )

#include "winlib/winlib.h"
#include "winlib/filesys.h"

#include <common/aliases.h>
#include <filesystem>

using namespace drjuke;
using namespace drjuke::winlib;

TEST(winlib, ExceptionRegular) try
{
    throw WindowsException("error opening file", ERROR_ACCESS_DENIED);
}
catch (const WindowsException& ex)
{
    std::cout << ex.what() << std::endl;
    std::cout << ex.dumpStackTrace() << std::endl;
    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(winlib, GetDesktopDirectory) try
{
    auto path = winlib::filesys::GetDesktopDirectory();
    std::cout << path << std::endl;
    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}