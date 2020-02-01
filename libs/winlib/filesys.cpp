#include "filesys.h"
#include "raii.h"
#include "windows_exception.h"

#include <filesystem>
#include <shlobj.h>

#undef DeleteFile
#undef CreateFile

// ReSharper disable CppInconsistentNaming
namespace drjuke::winlib::filesys
{
    void createFile(const Path &file)
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
            throw WindowsException("Can't create file");
        
    }

    void deleteFile(const Path &file)
    {
        if (!fs::exists(file))
            return;

        auto status = DeleteFileW(file.generic_wstring().c_str());

        if (!status)
            throw WindowsException("Can't delete file");
    }

    void appendFile(const Path &file, const std::string &data)
    {
        //if (!fs::exists(file))
        //{
        //    createFile(file);
        //}

        UniqueHandle file_handle(CreateFileW
        (
            file.generic_wstring().c_str(), // filename
            FILE_APPEND_DATA,               // open for writing
            FILE_SHARE_READ,                // allow multiple readers
            nullptr,                        // no security
            OPEN_ALWAYS,                    // open or create
            FILE_ATTRIBUTE_NORMAL,          // no attr. template
            nullptr
        ));

        if (file_handle.get() == INVALID_HANDLE_VALUE)
            throw WindowsException("Can't open file");

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
            throw WindowsException("Can't append file");
    }

    void createDirectory(const Path& path)
    {
        if (fs::exists(path))
            return;

        auto status = CreateDirectoryW
        (
            path.generic_wstring().c_str(),
            nullptr
        );

        if (!status)
            throw WindowsException("Can't create directory");
    }

    Path getDesktopDirectory()
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
            throw WindowsException("can't get desktop directory", status);

        return Path(path);
    }
}
// ReSharper restore CppInconsistentNaming