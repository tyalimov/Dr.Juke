#pragma once

#include "base_analyzer.h"
#include <filesystem>

#include <undecorate.h>

// Необходимо слинковать все библиотеки для работы с YARA.
#pragma warning( push )
#   pragma warning( disable : 4081 )
#   include <yaracpp.h>

LINK_YARA

namespace drjuke::scanlib
{
    class ClamAvReport final 
        : public BaseReport
    {
    public:
        explicit ClamAvReport
        (
            const std::vector<yaracpp::YaraRule>& rules
        );
    };

    class ClamAvAnalyzer final 
        : public BaseAnalyzer 
    {
    private:
        yaracpp::YaraDetector m_detector;
    
    public:
   
        BaseReportPtr getReport(const Path& path) override;
        void loadResources()                      override;
        std::string getName()                     override;
    };

    class ClamAvAnalyzerException final
        : public std::exception
    {
    public:
        [[nodiscard]] const char *what() const override final;
    };
}