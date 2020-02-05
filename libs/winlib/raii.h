#pragma once

#include <windows.h>
#include <memory>

#include <wintrust.h>
#include <Dbghelp.h>

#include <stdexcept>

namespace drjuke::winlib
{
    namespace
    {
        struct HandleDeleter
        {
            using pointer = HANDLE;

            void operator ()(HANDLE h) const
            {
                ::CloseHandle(h);
            }
        };

        struct RegHandleDeleter
        {
            using pointer = HKEY;

            void operator ()(HKEY h) const
            {
                ::RegCloseKey(h);
            }
        };

        struct SymHandleDeleter
        {
            using pointer = HANDLE;

            void operator ()(HANDLE h) const
            {
                ::SymCleanup(h);
            }
        };

        struct CertStoreDeleter
        {
            using pointer = HCERTSTORE;

            void operator ()(HANDLE h) const
            {
                // TODO: WTF?
                ::CloseHandle(h);
            }
        };

        void *AllocateBlob(size_t size)
        {
            void *ptr = malloc(size);

            if (!ptr)
            {
                throw std::runtime_error("can't allocate blob");
            }

            return ptr;
        }

        struct BlobDeleter
        {
            using pointer = void*;

            void operator ()(void *p) const
            {
                ::free(p);
            }
        };

    }
    using UniqueHandle    = std::unique_ptr< std::remove_pointer<HANDLE>::type,     HandleDeleter    >;
    using UniqueRegHandle = std::unique_ptr< std::remove_pointer<HKEY>::type,       RegHandleDeleter >;
    using UniqueSymHandle = std::unique_ptr< std::remove_pointer<HANDLE>::type,     SymHandleDeleter >;
    using UniqueCertStore = std::unique_ptr< std::remove_pointer<HCERTSTORE>::type, CertStoreDeleter >;
    using UniqueBlob      = std::unique_ptr< std::remove_pointer<void*>::type,      BlobDeleter      >;
}
