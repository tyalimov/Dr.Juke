#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )

#include <netlib/netlib.h>

using namespace  drjuke::netlib;

TEST(netlib, UpdateChecker_Regular) try
{
    auto update_checker = NetlibFactory::getUpdateChecker();
    auto hashes         = update_checker->getActualHashes();

    for (const auto& [key, value] : hashes)
    {
        std::cout
            << "file = " << key           << std::endl
            << "hash = " << value.first   << std::endl
            << "size = " << value.second  << std::endl
            << "-----------------"        << std::endl;
    }

    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(netlib, Updater_Regular) try
{
    auto update_checker = NetlibFactory::getUpdateChecker();
    auto hashes         = update_checker->getActualHashes();

    for (const auto& [key, value] : hashes)
    {
        std::cout
            << "file = " << key           << std::endl
            << "hash = " << value.first   << std::endl
            << "size = " << value.second  << std::endl
            << "-----------------"        << std::endl;
    }

    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}