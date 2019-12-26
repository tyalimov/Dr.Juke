#pragma once

#include <windows.h>
#include <memory>

#include <wintrust.h>
#include <Dbghelp.h>

namespace drjuke::winlib
{
    namespace
    {
        struct HandleDeleter
        {
            using pointer = HANDLE;

            void operator ()(HANDLE h) const
            {
                CloseHandle(h);
            }
        };

        struct SymHandleDeleter
        {
            using pointer = HANDLE;

            void operator ()(HANDLE h) const
            {
                SymCleanup(h);
            }
        };

        struct CertStoreDeleter
        {
            using pointer = HCERTSTORE;

            void operator ()(HANDLE h) const
            {
                CloseHandle(h);
            }
        };
    }
    using UniqueHandle    = std::unique_ptr< std::remove_pointer<HANDLE>::type,     HandleDeleter    >;
    using UniqueSymHandle = std::unique_ptr< std::remove_pointer<HANDLE>::type,     SymHandleDeleter >;
    using UniqueCertStore = std::unique_ptr< std::remove_pointer<HCERTSTORE>::type, CertStoreDeleter >;
}