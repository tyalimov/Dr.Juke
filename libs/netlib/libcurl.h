#pragma once

#include <common/utils.h>

#include "curl_raii.h"

namespace drjuke::netlib
{
    class CurlGlobalInstance
    {
    public:
        CurlGlobalInstance()
        {
            curl_global_init(CURL_GLOBAL_ALL);
        }

        ~CurlGlobalInstance()
        {
            curl_global_cleanup();
        }
    };

    class CurlConsumer
    {
    protected:

        CurlGlobalInstance& m_library_instance;
        UniqueCurl          m_curl;

    public:
        
        virtual ~CurlConsumer() = default;

        CurlConsumer()
            : m_library_instance(singleton::Singleton<CurlGlobalInstance>::Instance())
            , m_curl(curl_easy_init())
        {
            if (!m_curl)
            {
                // TODO: own exceptions
                throw std::runtime_error("can't init curl");
            }
        }
    };
}