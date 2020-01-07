#include <windows.h>
#include <wintrust.h>
#include <memory>

#include <winlib/raii.h>
#include <common/constants.h>

using namespace drjuke::threading;


int main()
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

}