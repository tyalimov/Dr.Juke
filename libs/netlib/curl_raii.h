#pragma once

#include <curl/curl.h>

#include <memory>

namespace drjuke::netlib
{
    namespace
    {
        struct CurlDeleter
        {
            using pointer = CURL*;

            void operator ()(CURL* h) const
            {
                ::curl_easy_cleanup(h);
            }
        };
    }

     using UniqueCurl = std::unique_ptr< std::remove_pointer<CURL*>::type, CurlDeleter>;
}