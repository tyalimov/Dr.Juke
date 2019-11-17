#include "pch.h"

#include <filesystem>

#include <common/aliases.h>
#include <scanlib/scanlib.h>
#include <scanlib/signature_analyzer.h>

using namespace drjuke;
using namespace drjuke::scanlib;

TEST(scanlib, SignatureAnalyzer_Regular) 
{
    const Path target = R"(inputs\scanlib\SignatureAnalyzer\signed_exe.exe)";
    try
    {
        std::cout << fs::current_path() << std::endl;
        std::cout << fs::file_size(target) << std::endl;
        SignatureAnalyzer analyzer;

        analyzer.loadResources();
        auto report = analyzer.getReport(target)->toJson();

        std::cout << report.dump() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        FAIL();
    }
}