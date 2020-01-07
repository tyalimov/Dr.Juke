#pragma once

#include "raii.h"

namespace drjuke::winlib
{
    class IPCClient
    {
    private:
        UniqueHandle m_pipe;
    public:
    };
}