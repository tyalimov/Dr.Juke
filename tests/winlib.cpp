#include "pch.h"

#include <windows.h>
#include "winlib/raii.h"
#include "winlib/except.h"

using namespace drjuke;
using namespace drjuke::winlib;

TEST(winlib, ExceptionRegular)
{
    try
    {
        drjuke::winlib::UniqueHandle t(CreateFileW
        (
            L"123", 
            GENERIC_WRITE, 
            0, 
            NULL,
            CREATE_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL, 
            NULL
        ));

        if (t.get() == INVALID_HANDLE_VALUE)
        {
            throw WindowsException("error opening file");
        }

        FAIL();
    }
    catch (const WindowsException& ex)
    {
        std::cout << ex.dumpStackTrace() << std::endl;
        SUCCEED();
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        FAIL();
    }
    
}