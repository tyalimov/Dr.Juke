#include "i_analyzer.h"
#include "yara_analyzer.h"

#include <iostream>
#include <vector>
#include <memory>

#include <tasklib/tasklib.h>
#include <common/constants.h>

using namespace drjuke::scansvc;
using namespace drjuke::threading;

//program filename

int main(int argc, const char *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    const std::vector<IAnalyzerPtr> kAnalyzers
    {
        std::make_shared<YaraAnalyzer>()
    };

    try
    {
        for (auto &analyzer : kAnalyzers)
        {
            analyzer->loadResources();
            
            std::cout << "Analyzer - [" << analyzer->getName() << "]\n";
            std::cout << analyzer->getReport(argv[1])->toJson().dump() << "\n\n";
        }
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}