#include <common/constants.h>

#include "clam_av_analyzer.h"

using drjuke::constants::scanlib::kClamAvRulesLocation;

namespace drjuke::scanlib
{
    BaseReportPtr ClamAvAnalyzer::getReport(const Path &path)
    {
        auto status = m_detector.analyze(path.generic_string());

        if (!status)
        {
            throw ClamAvAnalyzerException();
        }

        return std::make_shared<ClamAvReport>(m_detector.getDetectedRules());
    }

    void ClamAvAnalyzer::loadResources()
    {
        // Загружаем все файлы из заданной директории в детектор
        for (const auto& dir_entry : DirIterator(kClamAvRulesLocation))
        {
            if (!dir_entry.is_directory() && dir_entry.path().extension() == ".yar")
            {
                m_detector.addRuleFile(dir_entry.path().generic_string());
            }
        }
    }

    std::string ClamAvAnalyzer::getName()
    {
        return "Packers analyzer";
    }

    const char *ClamAvAnalyzerException::what() const
    {
        return "Packers analyzer error";
    }

    ClamAvReport::ClamAvReport(const std::vector<yaracpp::YaraRule> &rules)
    {
        m_report["infected"]      = !rules.empty();
        m_report["total_matched"] = rules.size();

        for (const auto& rule : rules)
        {
            m_report["matched_malware"].push_back(rule.getName());
        }
    }
}
