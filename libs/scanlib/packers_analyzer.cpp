#include <common/constants.h>

#include "packers_analyzer.h"

using drjuke::constants::scanlib::kPeidRulesLocation;

namespace drjuke::scanlib
{
    BaseReportPtr PackersAnalyzer::getReport(const Path &path)
    {
        auto status = m_detector.analyze(path.generic_string());

        if (!status)
        {
            throw PackersAnalyzerException();
        }

        return std::make_shared<PackersReport>(m_detector.getDetectedRules());
    }

    void PackersAnalyzer::loadResources()
    {
        // Загружаем все файлы из заданной директории в детектор
        for (const auto& dir_entry : DirIterator(kPeidRulesLocation))
        {
            if (!dir_entry.is_directory() && dir_entry.path().extension() == ".yar")
            {
                m_detector.addRuleFile(dir_entry.path().generic_string());
            }
        }
    }

    std::string PackersAnalyzer::getName()
    {
        return "Packers analyzer";
    }

    const char *PackersAnalyzerException::what() const
    {
        return "Packers analyzer error";
    }

    PackersReport::PackersReport(const std::vector<yaracpp::YaraRule> &rules)
    {
        m_report["infected"]      = !rules.empty();
        m_report["total_matched"] = rules.size();

        for (const auto& rule : rules)
        {
            m_report["matched_packers"].push_back(rule.getName());
        }
    }
}
