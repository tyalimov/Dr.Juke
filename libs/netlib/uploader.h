#pragma once

#include "libcurl.h"
#include "iuploader.h"

#include <loglib/loglib.h>

namespace drjuke::netlib
{
    struct FileDeleter
    {
        using pointer = FILE*;

        void operator ()(pointer f) const
        {
            ::fclose(f);
        }
    };

    struct CurlListDeleter
    {
        using pointer = curl_slist*;

        void operator ()(pointer l) const
        {
            ::curl_slist_free_all(l);
        }
    };

    using UniqueFile     = std::unique_ptr< std::remove_pointer<FILE*>::type,       FileDeleter>;
    using UniqueCurlList = std::unique_ptr< std::remove_pointer<curl_slist*>::type, CurlListDeleter>;

    class Uploader final
        : public CurlConsumer
        , public IUploader
    {
    public:

        explicit Uploader(const Path& file);

        void upload() override final;

    private:

        DECLARE_CLASS_LOGGER();

        Path           m_file_name;
        UniqueFile     m_file_ptr;
        UniqueCurlList m_header_list;
        std::string    m_url;

        static size_t on_read_data(void* ptr, size_t size, size_t nmemb, void* stream);
    };
}
