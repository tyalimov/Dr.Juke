#include "pch.h"

#include <filesystem>

#include <common/aliases.h>

#include <scanlib/scanlib.h>
//#include <scanlib/scanlib.h>
//#include <scanlib/signature_analyzer.h>

using namespace drjuke;
using namespace drjuke::scanlib;

TEST(scanlib, SignatureAnalyzer_Regular) 
{
    const Path target = R"(scanlib\SignatureAnalyzer\signed_exe.exe)";

    EXPECT_TRUE(fs::exists(target));

    try
    {
        auto analyzer = 
            AnalyzerFactory::get(AnalyzerFactory::AnalyzerId::kDigitalSignature);

        analyzer->loadResources();
        auto report = analyzer->getReport(target)->makeJson();

        std::cout << report.dump() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        FAIL();
    }
}

TEST(scanlib, SignatureAnalyzer_Signed) 
{
    const Path target = R"(scanlib\SignatureAnalyzer\calc.exe)";

    EXPECT_TRUE(fs::exists(target));

    try
    {
        auto analyzer = 
            AnalyzerFactory::get(AnalyzerFactory::AnalyzerId::kDigitalSignature);

        analyzer->loadResources();
        auto report = analyzer->getReport(target)->makeJson();

        std::cout << report.dump() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        FAIL();
    }
}