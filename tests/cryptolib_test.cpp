#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )


#include <common/aliases.h>
#include <cryptolib/cryptolib.h>

#include <filesystem>

using drjuke::cryptolib::Factory;

TEST(cryptolib, SHA512_Regular) try
{
    auto cryptor = Factory::getCryptor();
    auto file = R"(cryptolib\test.txt)";

    EXPECT_TRUE(fs::exists(file));

    std::cout << cryptor->sha512(file) << std::endl;

    SUCCEED();
}
catch (const std::exception &ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}