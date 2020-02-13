#include "yara_analyzer.h"

#include "settingslib/settingslib.h"


namespace drjuke::scanlib
{
    BaseReportPtr YaraAnalyzer::getReport(const Path &path)
    {
        auto status = m_detector.analyze(path.generic_string());

        if (!status)
        {
            throw YaraAnalyzerException();
        }

        return std::make_shared<YaraReport>(m_detector.getDetectedRules());
    }

    void YaraAnalyzer::loadResources()
    {
        Path rules_location = settingslib::Factory::getSettingsManager()->getResourcesDirectory();
        rules_location /= "yara";

        // Загружаем все файлы из заданной директории в детектор
        for (const auto& dir_entry : DirIterator(rules_location))
        {
            if (!dir_entry.is_directory() && dir_entry.path().extension() == ".yar")
            {
                m_detector.addRuleFile(dir_entry.path().generic_string());
            }
        }
    }

    std::string YaraAnalyzer::getName()
    {
        return "Yara analyzer";
    }

    const char *YaraAnalyzerException::what() const
    {
        return "YARA error";
    }

    YaraReport::YaraReport(const std::vector<yaracpp::YaraRule> &rules)
    {
        m_report["infected"]  = !rules.empty();
        m_report["name"]      = "Yara";
    }
}
