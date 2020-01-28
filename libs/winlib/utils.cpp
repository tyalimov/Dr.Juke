#include "utils.h"
#include "windows_exception.h"

#include <windows.h>
#include <lmcons.h>

namespace drjuke::winlib::utils
{
    [[nodiscard]] std::wstring GetCurrentUserName()
    {
        wchar_t user_name[UNLEN + 1]{0};
        DWORD size = UNLEN + 1;
        
        auto status = ::GetUserNameW(user_name, &size);

        if (!status)
        {
            throw WindowsException("Can't get user name");
        }

        return std::wstring(user_name);
    }

    [[nodiscard]] SYSTEMTIME GetCurrentSystemTime()
    {
        SYSTEMTIME now;
        ::GetLocalTime(&now);

        return now;
    }
}
