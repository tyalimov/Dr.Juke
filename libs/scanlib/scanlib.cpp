// Главный хедер библиотеки
#include "scanlib.h"

#include "base_analyzer.h"
//#include "yara_analyzer.h"
#include "signature_analyzer.h"

// заинклудить классификатор
// заинклудить ClamAV

namespace drjuke::scanlib
{
    BaseAnalyzerPtr AnalyzerFactory::get(AnalyzerId id)
    {
        constexpr auto analyzers_count = 5;
        static std::array<BaseAnalyzerPtr, analyzers_count> analyzers
        {
            // TODO: Заполнить необходимыми анализаторами
            std::make_shared<DigitalSignatureAnalyzer>(), // AnalyzerId::kYara
            std::make_shared<DigitalSignatureAnalyzer>(), // AnalyzerId::kClamAvSignature
            std::make_shared<DigitalSignatureAnalyzer>(), // AnalyzerId::kDigitalSignature
            std::make_shared<DigitalSignatureAnalyzer>(), // AnalyzerId::kPack
            std::make_shared<DigitalSignatureAnalyzer>(), // AnalyzerId::kAiClassifier
        };

        return analyzers[ToUnderlying(id)];
    }
}