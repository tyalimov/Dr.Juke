#pragma once

#include <undecorate.h>

//#pragma comment ( lib, BOOST_UNDECORATE_LIBRARY("locale") )

#include "base_analyzer.h"

#include <type_traits>

// TODO: Перенести в более подходящее место
template <typename T> 
typename std::underlying_type<T>::type constexpr ToUnderlying(const T& val) noexcept
{
    return static_cast<typename std::underlying_type<T>::type>(val);
}

namespace drjuke::scanlib
{
    class AnalyzerFactory
    {
    public:

        enum class AnalyzerId
        { 
            kYara,              // Анализатор YARA правил
            kClamAvSignature,   // Анализатор сигнатур ClamAV
            kDigitalSignature,  // Анализатор ЭЦП
            kPack,              // Анализатор упакованных файлов
            kAiClassifier       // Анализатор на основе классификатора
        };

        [[nodiscard]] static BaseAnalyzerPtr get(AnalyzerId id);
    };
}