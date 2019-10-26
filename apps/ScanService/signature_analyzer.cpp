#include "signature_analyzer.h"

namespace drjuke::scansvc
{
    IReportPtr SignatureAnalyzer::getReport(const Path &path)
    {
    }

    void SignatureAnalyzer::loadResources()
    {
    }

    std::string SignatureAnalyzer::getName()
    {
        return "Signature";
    }
}