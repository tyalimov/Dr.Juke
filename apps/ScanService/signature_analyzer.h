#pragma once

#include "i_analyzer.h"
#include <filesystem>

#include <common/aliases.h>

namespace drjuke::scansvc
{
    class SignatureReport : public IReport
    {
    public:
        SignatureReport
        (

        );
    };

    class SignatureAnalyzer : public IAnalyzer
    {
    public:

        IReportPtr getReport(const Path& path) override;
        void loadResources()                   override;
        std::string getName()                  override;
    };
}