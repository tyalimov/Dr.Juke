#include "cryptor.h"

#include <channels.h>
#include <filters.h>
#include <files.h>
#include <sha.h>
#include <hex.h>

#include <filesystem>

namespace drjuke::cryptolib
{    
    std::string Cryptor::sha512(const Path& file)
    {
        std::string           result;
        CryptoPP::SHA512      sha512;
        CryptoPP::HashFilter  filter(sha512, 
                                     new CryptoPP::HexEncoder
                                     (
                                         new CryptoPP::StringSink(result)
                                     ));

        CryptoPP::ChannelSwitch channel_switch(filter);

        CryptoPP::FileSource(file.generic_string().c_str(), 
                             true, 
                             new CryptoPP::Redirector(channel_switch));

        return result;
    }
}
