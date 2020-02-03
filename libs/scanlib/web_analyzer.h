#pragma once

#include "base_analyzer.h"
#include <filesystem>


#pragma warning( push )
#   pragma warning( disable : 4081 )
#   include <yara/yaracpp.h>



namespace drjuke::scanlib
{
    class WebReport final 
        : public BaseReport
    {
    public:
        explicit WebReport
        (
            const std::vector<yaracpp::YaraRule>& rules
        );
    };

    class WebAnalyzer final 
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
