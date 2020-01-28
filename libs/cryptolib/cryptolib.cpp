#include "cryptolib.h"

#include "cryptor.h"

namespace drjuke::cryptolib
{
    ICryptorPtr Factory::getCryptor() 
    {
        return std::make_unique<Cryptor>();
    }
}