#include "yara_analyzer.h"

#include <common/constants.h>

using drjuke::constants::scansvc::kYaraRulesLocation;

namespace drjuke::scansvc
{
    IReportPtr YaraAnalyzer::getReport(const Path &path)
    {
        m_detector.analyze(path.generic_string());
        
        return std::make_shared<YaraReport>(m_detector.getDetectedRules());
    }

	void YaraAnalyzer::loadResources()
	{
        // Загружаем все файлы из заданной директории в детектор
        for (const auto &dir_entry : DirIterator(kYaraRulesLocation))
        {
            if (!dir_entry.is_directory() && dir_entry.path().extension() == ".yar")
            {
                // TODO: залоггировать
                m_detector.addRuleFile(dir_entry.path().generic_string());
            }
        }
	}

    std::string YaraAnalyzer::getName()
    {
		return "YARA";
    }

	YaraReport::YaraReport(const std::vector<yaracpp::YaraRule> &rules)
    {
        m_report["infected"]      = !rules.empty();
        m_report["total_matched"] = rules.size();

        for (const auto &rule : rules)
        {
            m_report["matched_rules"].push_back(rule.getName());
        }
    }
}
