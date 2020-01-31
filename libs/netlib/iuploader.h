#pragma once

#include <common/aliases.h>

namespace drjuke::netlib
{
    class IUploader
    {
    public:
        virtual ~IUploader()  = default;
        virtual void upload() = 0;
    };

    using UploaderPtr = std::unique_ptr<IUploader>;
}