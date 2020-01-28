#include "filesys.h"
#include "raii.h"
#include "windows_exception.h"

#include <filesystem>
#include <shlobj.h>

namespace drjuke::winlib::filesys
{
    void CreateNewFile(const Path &file)
    {
        UniqueHandle file_handle(CreateFileW
        (
            file.generic_wstring().c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        ));

        if (file_handle.get() == INVALID_HANDLE_VALUE)
        {
            throw WindowsException(("Can't create file - " + file.generic_string()).c_str());
        }
    }
    void AppendFile(const Path &file, const std::string &data)
    {
        if (!fs::exists(file))
        {
            CreateNewFile(file);
        }

        UniqueHandle file_handle(CreateFileW
        (
            file.generic_wstring().c_str(),
            FILE_APPEND_DATA | GENERIC_WRITE,
            0,
            nullptr,
            TRUNCATE_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        ));

        if (file_handle.get() == INVALID_HANDLE_VALUE)
        {
            throw WindowsException(("Can't open file - " + file.generic_string()).c_str());
        }

        DWORD bytes_written{ 0 };

        auto status = WriteFile
        (
            file_handle.get(),
            data.c_str(),
            data.size(),
            &bytes_written,
            nullptr
        );

        if (status == FALSE || data.size() != bytes_written)
        {
            throw WindowsException(("Error appending to file - " + file.generic_string()).c_str());
        }
    }

    Path GetDesktopDirectory()
    {
        static wchar_t path[ MAX_PATH + 1 ]{0};

        auto status = SHGetFolderPathW
        (
            nullptr, 
            CSIDL_DESKTOP,
            nullptr, 
            0, 
            path
        );

        if (status != S_OK)
        {
            throw WindowsException("can't get desktop directory", status);
        }

        return Path(path);
    }
}
