#pragma once

namespace drjuke::netlib
{
    class IUpdater
    {
    public:
        virtual ~IUpdater() = default;
        virtual void downloadFiles() = 0;
    };

    using UpdaterPtr = std::unique_ptr<IUpdater>;
}