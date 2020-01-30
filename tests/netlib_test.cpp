#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )

#include <netlib/netlib.h>
#include "utils.h"

using namespace  drjuke::netlib;

TEST(netlib, UpdateChecker_Regular) try
{
    InitialiseEnvironment();

    auto update_checker = Factory::getUpdateChecker();
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
    InitialiseEnvironment();

    std::vector<drjuke::Path> files
    {
        "test_1.txt",
        "test_2.txt"
    };

    auto progress_bar = std::make_shared<LoadingProgress>();
    auto updater = Factory::getUpdater(files, "loaded", progress_bar);

    updater->downloadFiles();

    SUCCEED();
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}