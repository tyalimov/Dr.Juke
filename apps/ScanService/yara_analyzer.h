#pragma once

#include "yara_engine.h"
#include "i_analyzer.h"
#include <filesystem>

#define FIELD_RESULT         "infected" 
#define FIELD_TOTAL_MATCHED  "total_matched"
#define FIELD_MATCHED_LIST   "matched_rules"

namespace drjuke::scansvc
{
/*
     Формат отчета от анализатора:
     {
         "infected": true/false ----- Было ли найдено совпадение в ходе проверки
         "total_matched" : <число> -- По скольким правилам было найдено совпадение
         "matched_rules" : ---------- Список имен правил, для которых было найдено совпадение
         [
            "имя", ------------------ Имя правила
            "имя"  ------------------ Имя правила
         ]
     }
*/
    class YaraReport final : public IReport
    {
    public:
        explicit YaraReport
        (
			const std::vector<yaracpp::YaraRule>& rules
		);
    };

    class YaraAnalyzer final : public IAnalyzer 
    {
    private:
        yaracpp::YaraDetector m_detector;
    
    public:
   
        IReportPtr getReport(const Path& path) override;
        void loadResources()                   override;
        std::string getName()                  override;
    };

}