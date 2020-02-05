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
    auto path = winlib::filesys::getDesktopDirectory();
    std::cout << path << std::endl;
    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(winlib, kernelFilePathExists) try
{
    std::wstring path = L"c:\\Windows\\System32\\calc.exe";
    path = winlib::utils::getFileKernelPath(path);
    std::wcout << "Calculator path: " << path << std::endl;
    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(winlib, kernelFilePathNotExist) try
{
    std::wstring path = L"c:\\Windows\\System32\\calc1.exe";
    path = winlib::utils::getFileKernelPath(path);
    FAIL();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    SUCCEED();
}

TEST(winlib, kernelFilePathDirectory) try
{
    std::wstring path = L"c:\\Windows\\System32";
    path = winlib::utils::getFileKernelPath(path);
    std::wcout << "System32 path: " << path << std::endl;
    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}


TEST(winlib, kernelKeyPathExists) try
{
    std::wstring path = L"Control Panel\\Desktop\\Colors";
    path = winlib::utils::getKeyKernelPath(HKEY_CURRENT_USER, path);
    std::wcout << "Desktop colors path: " << path << std::endl;
    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(winlib, kernelKeyPathNotExist) try
{
    std::wstring path = L"Control Panel\\Desktop\\Colors12345";
    path = winlib::utils::getKeyKernelPath(HKEY_CURRENT_USER, path);
    FAIL();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    SUCCEED();
}
