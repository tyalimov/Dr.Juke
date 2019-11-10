#include <memory>

#include <windows.h>

namespace drjuke
{

#if 0
    class SafeHandle: public std::unique_ptr<std::remove_pointer<HANDLE>::type,void(*)( HANDLE )>
    {
    public:
        SafeHandle( HANDLE handle ): unique_ptr( handle, &SafeHandle::close )
        {
        }
        operator HANDLE()
        {
            return get();
        }
        const bool valid() const
        {
            return ( get() != INVALID_HANDLE_VALUE );
        }
    private:
        static void close( HANDLE handle )
        {
            if ( handle != INVALID_HANDLE_VALUE )
                CloseHandle( handle );
        }
    };

    template <typename T, typename Y >
    class MyTemplate {
        T element;
    public:
        MyTemplate (T arg) {element=arg;}
        T divideBy2 () {return element/2;}
    };

    MyTemplate<int, int> A;

    template <typename ResourceT, typename DeleterT>
    class Unique
    {
    private:
        std::unique_ptr<std::remove_pointer<ResourceT>::type, void(*)(ResourceT)> ptr;
    public:
        Unique(ResourceT res)
            : unique_ptr(res, &DeleterT::operator())
        {}

        operator ResourceT()
        {
            return ptr.get();
        }

        const bool valid() const
        {
            return ( ptr.get() != INVALID_HANDLE_VALUE );
        }
    }

    struct HandleDeleter
    {
        void operator()(HANDLE handle)
        {
            ::CloseHandle(handle)
        }
    };

    using UniqueHandle = Unique<HANDLE, HandleDeleter>;
    //using SharedHandle = std::shared_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;
#endif
}