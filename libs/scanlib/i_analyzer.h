#pragma once

#pragma warning( push , 0 )
#   include <json.hpp>
#pragma warning( pop )

#include <filesystem>
#include <string>

#include <common/aliases.h>

// TODO: переименовать в Base[имя], так как интерфейс здесь только анализатор

namespace drjuke::scanlib
{
    // TODO: Добавить виртуальный метод, осуществляющий первичную инициализацию json
    class BaseReport
    {
    protected:
        Json m_report;
    public:
        virtual ~BaseReport() = default;
        [[nodiscard]] virtual Json toJson() { return m_report; }
    };

    using BaseReportPtr = std::shared_ptr<BaseReport>;

    class BaseAnalyzer
    {
    public:
        virtual ~BaseAnalyzer()                           = default;
        virtual BaseReportPtr getReport(const Path &path) = 0;
        virtual void loadResources()                   = 0;
        virtual std::string getName()                  = 0;
    };

    using BaseAnalyzerPtr = std::shared_ptr<BaseAnalyzer>;
}