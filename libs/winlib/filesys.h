#pragma once
#include <common/aliases.h>

// Здесь была изменена конвенция именования
// из-за конфликтов с макросами из windows.h
namespace drjuke::winlib::filesys
{
    // ReSharper disable CppInconsistentNaming

    void createFile(const Path& file);
    void deleteFile(const Path& file);
    void appendFile(const Path& file, const std::string& data);
    Path getDesktopDirectory();

    // ReSharper restore CppInconsistentNaming
}
