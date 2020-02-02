#include "updater_utils.h"

#include <cryptolib/cryptolib.h>
#include <boost/algorithm/string.hpp>

[[nodiscard]] std::map<std::string, std::string> GetFilesHashMap(const Path& directory)
{
    std::map<std::string, std::string> result;
    
    auto cryptor  = cryptolib::Factory::getCryptor();

    for (const auto& file : DirIterator(directory))
    {
        if (!file.is_directory())
        {
            result[file.path().filename().generic_string()]
                = cryptor->sha512(file.path());
        }
    }

    return result;
}

[[nodiscard]] std::map<std::string, std::pair<std::string, uint32_t>> GetFilesToDownload
(
    const std::map<std::string, std::string>&                      local_hashmap,
    const std::map<std::string, std::pair<std::string, uint32_t>>& remote_hashmap
)
{
    // Удалить из remote_hashmap те файлы, что совпадают с local_hashmap
    
    auto result = remote_hashmap;
    
    for (const auto& [key, value] : local_hashmap)
    {
        if (result.find(key) != result.end())
        {
            if (boost::iequals(value, result.at(key).first))
            {
                result.erase(key);
            }
        }
    }

    return result;
}