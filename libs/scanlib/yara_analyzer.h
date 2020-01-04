#pragma once

#include "base_analyzer.h"
#include <filesystem>

// Необходимо слинковать все библиотеки для работы с YARA.
#pragma warning( push )
#   pragma warning( disable : 4081 )
#   include <yaracpp.h>

#define FIELD_RESULT         "infected" 
#define FIELD_TOTAL_MATCHED  "total_matched"
#define FIELD_MATCHED_LIST   "matched_rules"

/*
    Анализатор файлов, работающий на основе Yara правил.
    Формат отчета от анализатора:
    {
        "infected": true/false ----- Было ли найдено совпадение в ходе проверки
        "total_matched" : <число> -- По скольким правилам было найдено совпадение
        "matched_rules" : ---------- Список имен правил, для которых было найдено совпадение
        [
            "имя", ----------------- Имя правила
            "имя"  ----------------- Имя правила
        ]
    }
*/

namespace drjuke::scanlib
{
    class YaraReport final 
        : public BaseReport
    {
    public:
        explicit YaraReport
        (
            const std::vector<yaracpp::YaraRule>& rules
        );
    };

    class YaraAnalyzer final 
        : public BaseAnalyzer 
    {
    private:
        yaracpp::YaraDetector m_detector;
    
    public:
   
        BaseReportPtr getReport(const Path& path) override;
        void loadResources()                      override;
        std::string getName()                     override;
    };

    class YaraAnalyzerException final
        : public std::exception
    {
    public:
        [[nodiscard]] const char *what() const override final;
    };
}