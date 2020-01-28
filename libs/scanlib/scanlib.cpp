#include <common/utils.h> // ToUnderlying


#include "scanlib.h" // Главный хедер библиотеки
#include "base_analyzer.h" // Интерфейсы

#include "certificate_analyzer.h"
#include "yara_analyzer.h"
#include "packers_analyzer.h"
#include "clam_av_analyzer.h"

namespace drjuke::scanlib
{
    std::vector<BaseAnalyzerPtr> Factory::m_analyzers 
    {
        std::make_shared<YaraAnalyzer>(),             // AnalyzerId::kYara
        std::make_shared<ClamAvAnalyzer>(),           // AnalyzerId::kClamAvSignature
        std::make_shared<DigitalSignatureAnalyzer>(), // AnalyzerId::kDigitalSignature
        std::make_shared<PackersAnalyzer>(),          // AnalyzerId::kPack
    };

    auto Factory::getAnalyzer(AnalyzerId id) -> decltype(m_analyzers)::value_type
    {
        return m_analyzers[ToUnderlying(id)];
    }

    auto Factory::getAll() -> decltype(m_analyzers)
    {
        return m_analyzers;
    }
}
