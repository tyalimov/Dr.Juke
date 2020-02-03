#include "web_analyzer.h"
#include "settingslib/settingslib.h"


namespace drjuke::scanlib
{
    BaseReportPtr WebAnalyzer::getReport(const Path &path)
    {
        auto status = m_detector.analyze(path.generic_string());

        if (!status)
        {
            throw PackersAnalyzerException();
        }

        return std::make_shared<WebReport>(m_detector.getDetectedRules());
    }

    void WebAnalyzer::loadResources()
    {
        Path rules_location = settingslib::Factory::getSettingsManager()->getResourcesDirectory();
        rules_location /= "web";

        // Загружаем все файлы из заданной директории в детектор
        for (const auto& dir_entry : DirIterator(rules_location))
        {
            if (!dir_entry.is_directory() && dir_entry.path().extension() == ".yar")
            {
                m_detector.addRuleFile(dir_entry.path().generic_string());
            }
        }
    }

    std::string WebAnalyzer::getName()
    {
        return "Packers analyzer";
    }

    const char *PackersAnalyzerException::what() const
    {
        return "Packers analyzer error";
    }

    WebReport::WebReport(const std::vector<yaracpp::YaraRule> &rules)
    {
        m_report["infected"]      = !rules.empty();
        m_report["total_matched"] = rules.size();

        for (const auto& rule : rules)
        {
            m_report["matched_packers"].push_back(rule.getName());
        }
    }
}
