#pragma once

#pragma warning( push , 0 )
#   include <json/json.hpp>
#pragma warning( pop )

#include <filesystem>
#include <string>

#include <common/aliases.h>

namespace drjuke::scanlib
{
    // TODO: Добавить виртуальный метод, осуществляющий первичную инициализацию json
    class BaseReport
    {
    protected:
        Json m_report;
    public:
        virtual ~BaseReport() = default;
        [[nodiscard]] 
        virtual Json makeJson() { return m_report; }
    };

    using BaseReportPtr = std::shared_ptr<BaseReport>;

    class BaseAnalyzer
    {
    public:
        virtual ~BaseAnalyzer()                           = default;
        virtual BaseReportPtr getReport(const Path &path) = 0;
        virtual void loadResources()                      = 0;
        virtual std::string getName()                     = 0;
 
#if 0 // TODO: Внедрить
          BaseReportPtr analyze()
        {
            loadResources();
            return getReport();
        }
#endif

    };

    using BaseAnalyzerPtr = std::shared_ptr<BaseAnalyzer>;
}