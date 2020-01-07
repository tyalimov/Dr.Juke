#pragma once

#include "icryptor.h"

namespace drjuke::cryptolib
{
    class Cryptor final
        : public ICryptor
    {
    public:
        std::string sha512(const Path &file) override final;
    };
}