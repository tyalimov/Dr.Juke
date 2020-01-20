#include <exception>
#include <common/utils.h>

#include "avservice.h"

using drjuke::svc::AVService;

int main() try
{
    AVService& service_instance = singleton::Singleton<AVService>::Instance();
    service_instance.start();
}
catch (const std::exception& /*ex*/)
{
    // TODO: log << ex.what() << std::endl;
    exit(1);
}
catch (...)
{
    exit(2);
}