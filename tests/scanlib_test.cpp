#pragma warning (push, 0)
#   pragma warning (disable : 26812)
#   pragma warning (disable : 28020)
#   pragma warning (disable : 26495)
#   pragma warning (disable : 26444)
#   include "gtest/gtest.h"
#pragma warning ( pop )


#include <common/aliases.h>
#include <scanlib/scanlib.h>

#include <filesystem>

using namespace drjuke;
using namespace drjuke::scanlib;

TEST(scanlib, SignatureAnalyzer_Regular) try
{
    const Path target = R"(scanlib\SignatureAnalyzer\signed_exe.exe)";

    EXPECT_TRUE(fs::exists(target));

    auto analyzer =  Factory::getAnalyzer(Factory::AnalyzerId::kDigitalSignature);

    analyzer->loadResources();
    auto report = analyzer->getReport(target)->makeJson();

    std::cout << report.dump() << std::endl;

    // TODO: Проверить ожидаемые значения
    SUCCEED();
}
catch (const std::exception &ex)
{
    std::cerr << ex.what() << std::endl;
    FAIL();
}

TEST(scanlib, SignatureAnalyzer_Signed)  try
{
    const Path target = R"(scanlib\SignatureAnalyzer\calc.exe)";

    EXPECT_TRUE(fs::exists(target));

    auto analyzer = Factory::getAnalyzer(Factory::AnalyzerId::kDigitalSignature);

    analyzer->loadResources();
    auto report = analyzer->getReport(target)->makeJson();

    std::cout << report.dump() << std::endl;

    // TODO: Проверить ожидаемые значения
    SUCCEED();
}
catch (const std::exception &ex)
{
    std::cerr << ex.what() << std::endl;
    FAIL();
}

TEST(scanlib, Yara) try
{
    const Path target = R"(scanlib\SignatureAnalyzer\calc.exe)";

    EXPECT_TRUE(fs::exists(target));

    auto analyzer = Factory::getAnalyzer(Factory::AnalyzerId::kYara);

    analyzer->loadResources();
    auto report = analyzer->getReport(target)->makeJson();

    std::cout << report.dump() << std::endl;

    // TODO: Проверить ожидаемые значения
    SUCCEED();
}
catch (const std::exception &ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(scanlib, ClamAV) try
{
    const Path target = R"(scanlib\SignatureAnalyzer\calc.exe)";

    EXPECT_TRUE(fs::exists(target));

    auto analyzer = Factory::getAnalyzer(Factory::AnalyzerId::kClamAvSignature);

    analyzer->loadResources();
    auto report = analyzer->getReport(target)->makeJson();

    std::cout << report.dump() << std::endl;

    // TODO: Проверить ожидаемые значения
    SUCCEED();
}
catch (const std::exception &ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}

TEST(scanlib, PEiD) try
{
    const Path target = R"(scanlib\SignatureAnalyzer\calc.exe)";

    EXPECT_TRUE(fs::exists(target));

    auto analyzer = Factory::getAnalyzer(Factory::AnalyzerId::kPack);

    analyzer->loadResources();
    auto report = analyzer->getReport(target)->makeJson();

    std::cout << report.dump() << std::endl;

    // TODO: Проверить ожидаемые значения
    SUCCEED();
}
catch (const std::exception &ex)
{
    std::cout << ex.what() << std::endl;
    FAIL();
}