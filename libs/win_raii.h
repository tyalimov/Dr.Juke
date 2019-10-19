#include <windows.h>
#include <memory>

namespace drjuke
{
	struct HandleDeleter
	{
		void operator()(HANDLE handle)
		{
			::CloseHandle(handle)
		}
	}

	using UniqueHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;
	using SharedHandle = std::shared_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;
}