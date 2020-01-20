#include "avservice.h"

namespace drjuke::svc
{
    DWORD AVService::run()
    {
        LOG_INFO() << L"Service started" << std::endl;
        return __super::run();
    }

    void AVService::onShutdown() noexcept
    {
    }
}
