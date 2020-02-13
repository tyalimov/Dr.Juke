#pragma once

#include <common/aliases.h>

#include <string>
#include <map>
#include <utility>

#pragma warning(push)
#pragma warning(disable:26451)
#include <json/json.hpp>
#pragma warning(pop)

namespace drjuke::netlib
{
    class ICloudScanner
    {
    public:
        virtual ~ICloudScanner() = default;
        [[nodiscard]] virtual Json scanFile(const Path& filename) const = 0;
    };

    using CloudScannerPtr = std::unique_ptr<ICloudScanner>;
}