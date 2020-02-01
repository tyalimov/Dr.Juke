#pragma once

namespace drjuke::netlib
{
    class IUpdater
    {
    public:
        virtual ~IUpdater()         = default;
        virtual void downloadFile() = 0;
    };

    using UpdaterPtr = std::unique_ptr<IUpdater>;
}