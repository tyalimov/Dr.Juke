#include "yara_analyzer.h"
#include <iostream>

namespace drjuke::scansvc
{
	using DirIter = std::filesystem::recursive_directory_iterator;

    namespace
    {
        // TODO: Перенести в common.h
		const std::string kYaraRulesLocation = R"(D:\Dr.Juke_resources\CVE_Rules)";
    }


    AnalyzeReport YaraAnalyzer::analyze(const Path &path)
    {
		AnalyzeReport report;

		report["status"]              = false;
		report["total_rules_matched"] = 0;
		report["matched_rules"]       = Json::array();

		bool infected = m_detector.analyze(path.generic_string());

		report["status"] = infected ? "infected" : "normal";

        if (infected)
        {
            auto matched_rules = m_detector.getDetectedRules();
			report["total_rules_matched"] = matched_rules.size();

            for (const auto &rule : matched_rules)
            {
				report["matched_rules"].push_back(rule.getName());
            }
        }

		return report;
    }

	void YaraAnalyzer::prepare()
	{
        // Загружаем все файлы из заданной директории в детектор
        for (const auto &dir_entry : DirIter(kYaraRulesLocation))
        {
            if (!dir_entry.is_directory() && dir_entry.path().extension() == ".yar")
            {
				//std::cout << dir_entry.path() << std::endl;
				m_detector.addRuleFile(dir_entry.path().generic_string());
            }
        }
	}

    std::string YaraAnalyzer::getName()
    {
		return "YARA";
    }
}
