#pragma once
#include <common/aliases.h>

namespace drjuke::winlib::filesys
{
    void CreateNewFile(const Path& file);
    void AppendFile(const Path& file, const std::string& data);
}
