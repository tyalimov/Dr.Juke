#pragma once

#include "libcurl.h"

#include <loglib/loghlp.h>
#include "icloud_scanner.h"

namespace drjuke::netlib
{
    class CloudScanner final
        : public CurlConsumer
        , public ICloudScanner
    {
    public:

        struct Responce
        {
            std::string data;
        };

        [[nodiscard]] Json scanFile(const Path& filename) const override;

    private:

        DECLARE_CLASS_LOGGER();

        static size_t on_http_data(void* buffer, size_t size, size_t nmemb, void* stream);
        Responce m_responce;
    };
}
#pragma once
