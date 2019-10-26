#pragma once

#pragma warning( push , 0 )
#   include <json.hpp>
#pragma warning( pop )

#include <filesystem>
#include <string>

#include <common/aliases.h>

namespace drjuke::scansvc
{
    class IReport
    {
    protected:
        Json m_report;
    public:
        virtual ~IReport() = default;
        [[nodiscard]] virtual Json toJson() { return m_report; }
    };

    using IReportPtr = std::shared_ptr<IReport>;

    class IAnalyzer
    {
    public:
        virtual ~IAnalyzer()                           = default;
        virtual IReportPtr getReport(const Path &path) = 0;
        virtual void loadResources()                   = 0;
        virtual std::string getName()                  = 0;
    };

    using IAnalyzerPtr = std::shared_ptr<IAnalyzer>;
}