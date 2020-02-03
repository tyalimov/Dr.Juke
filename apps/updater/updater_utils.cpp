#include "updater_utils.h"

#include <cryptolib/cryptolib.h>
#include <boost/algorithm/string.hpp>
#include <loglib/loglib.h>

[[nodiscard]] std::map<std::string, std::string> GetFilesHashMap(const Path& directory)
{
    std::map<std::string, std::string> result;
    
    auto cryptor  = cryptolib::Factory::getCryptor();

    for (const auto& file : DirIterator(directory))
    {
        if (!file.is_directory())
        {
            auto path = fs::relative(file, directory);
            result[path.generic_string()] = cryptor->sha512(file.path());
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
        std::string processed_key { key };
        std::string to_replace    { R"(/)" };
        std::string replace_with  { R"(\)" };

        boost::replace_all(processed_key, to_replace, replace_with);

        if (result.find(processed_key) != result.end())
        {
            if (boost::iequals(value, result.at(processed_key).first))
            {
                result.erase(processed_key);
            }
        }
    }

    return result;
}
