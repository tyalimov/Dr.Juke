#pragma once

#include "base_analyzer.h"
#include <filesystem>


#pragma warning( push )
#   pragma warning( disable : 4081 )
#   include <yara/yaracpp.h>



namespace drjuke::scanlib
{
    class PackersReport final 
        : public BaseReport
    {
    public:
        explicit PackersReport
        (
            const std::vector<yaracpp::YaraRule>& rules
        );
    };

    class PackersAnalyzer final 
        : public BaseAnalyzer 
    {
    private:
        yaracpp::YaraDetector m_detector;
    
    public:
   
        BaseReportPtr getReport(const Path& path) override;
        void loadResources()                      override;
        std::string getName()                     override;
    };

    class PackersAnalyzerException final
        : public std::exception
    {
    public:
        [[nodiscard]] const char *what() const override final;
    };
}