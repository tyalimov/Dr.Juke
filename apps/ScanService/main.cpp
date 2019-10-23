#include "basic_analyzer.h"
#include "yara_analyzer.h"

#include <iostream>
#include <vector>
#include <memory>

using namespace drjuke::scansvc;

//program filename

int main(int argc, const char *argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	using BasicAnalyzerPtr = std::shared_ptr<BasicAnalyzer>;

	const std::vector<BasicAnalyzerPtr> kAnalyzers
	{
        std::make_shared<YaraAnalyzer>()
	};

	try
	{
		for (auto &analyzer : kAnalyzers)
		{
			analyzer->prepare();
            
			std::cout << "Analyzer - [" << analyzer->getName() << "]\n";
			std::cout << analyzer->analyze(argv[1]).dump() << "\n\n";
		}
	}
    catch (const std::exception &ex)
    {
		std::cout << ex.what() << std::endl;
    }

	return 0;
}