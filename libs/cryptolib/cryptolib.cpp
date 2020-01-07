#include "cryptolib.h"

#include "cryptor.h"


namespace drjuke::cryptolib
{
    ICryptorPtr CryptorFactory::get() 
    {
        return std::make_unique<Cryptor>();
    }
}