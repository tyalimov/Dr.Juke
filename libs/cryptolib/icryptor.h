#pragma once

#include <common/aliases.h>

#include <string>

namespace drjuke::cryptolib
{
    class ICryptor
    {
    public:
        virtual ~ICryptor() = default;
        virtual std::string sha512(const Path& file) = 0;
    };
}