#pragma once

#include "libcurl.h"

namespace drjuke::netlib
{
    class UpdateChecker final
        : public CurlConsumer
        , public IUpdateChecker
    {
    public:

        struct Responce
        {
            std::string data;
        };

        [[nodiscard]] std::map<std::string, std::pair<std::string, std::uint32_t>> getActualHashes() const override;

    private:

        static size_t on_http_data(void* buffer, size_t size, size_t nmemb, void* stream);
        Responce m_responce;
    };
}
