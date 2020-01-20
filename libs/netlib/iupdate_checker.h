#pragma once

#include <common/aliases.h>

#include <string>
#include <map>
#include <utility>

namespace drjuke::netlib
{
    class IUpdateChecker
    {
    public:
        virtual ~IUpdateChecker() = default;
        [[nodiscard]] virtual std::map<std::string, std::pair<std::string, std::uint32_t>> getActualHashes() const = 0;
    };

    using UpdateCheckerPtr = std::unique_ptr<IUpdateChecker>;
}