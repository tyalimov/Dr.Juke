#pragma once

#include "yara_engine.h"
#include "basic_analyzer.h"
#include <filesystem>

/*
     Вид отчета от анализатора:
    
    {
        status: "infected/normal"
        total_rules_matched : <число>
        matched_rules :
        [
            "имя",
            "имя"
        ]
    }

 
 */

namespace drjuke::scansvc
{
    // TODO: Добавить kYaraAnalyze в common.h

	class YaraAnalyzer : public BasicAnalyzer
	{
	private:
		yaracpp::YaraDetector m_detector;

	public:

		AnalyzeReport analyze(const Path &path) override;
		void prepare() override;
		std::string getName() override;
	};

}