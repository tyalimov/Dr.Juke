#pragma once

#include <undecorate.h>

#include "base_analyzer.h"

LINK_YARA

namespace drjuke::scanlib
{
    class Factory
    {
    public:

        enum class AnalyzerId
        { 
            kYara,              // Анализатор YARA правил
            kClamAvSignature,   // Анализатор сигнатур ClamAV
            kDigitalSignature,  // Анализатор ЭЦП
            kPack,              // Анализатор упакованных файлов
        };

        static std::vector<BaseAnalyzerPtr> m_analyzers;

        [[nodiscard]] static auto getAnalyzer(AnalyzerId id) -> decltype(m_analyzers)::value_type;
        [[nodiscard]] static auto getAll()                   -> decltype(m_analyzers);
    };
}