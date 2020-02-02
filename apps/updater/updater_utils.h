#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <filesystem>

#include <common/aliases.h>

using namespace drjuke;

[[nodiscard]] std::map<std::string, std::string> GetFilesHashMap
(
    const Path& directory
);

[[nodiscard]] std::map<std::string, std::pair<std::string, uint32_t>> GetFilesToDownload
(
    const std::map<std::string, std::string>& local_hashmap,
    const std::map<std::string, std::pair<std::string, uint32_t>>& remote_hashmap
);