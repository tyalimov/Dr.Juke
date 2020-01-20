#pragma once

#include <undecorate.h>
#include <memory>
#include "icryptor.h"

#pragma warning( push )                                   
#   pragma warning(disable : 4081)                         
    LINK_LIBRARY("cryptopp\\cryptlib")
#pragma warning( pop ) 

namespace drjuke::cryptolib
{
    using ICryptorPtr = std::unique_ptr<ICryptor>;
    class CryptorFactory
    {
    public:
        [[nodiscard]] static ICryptorPtr get();
    };
}