#include <type_traits>

template <typename T> 
typename std::underlying_type<T>::type constexpr ToUnderlying(const T& val) noexcept
{
    return static_cast<typename std::underlying_type<T>::type>(val);
}