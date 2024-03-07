#ifndef LIBCPP_UTILITY
#define LIBCPP_UTILITY

#include <library/libcpp/type_traits.hpp>

namespace std
{
    template<typename T>
    typename std::remove_reference<T>::type &&move(T &&t) noexcept
    {
        return static_cast<typename std::remove_reference<T>::type &&>(t);
    }

    template<typename T>
    constexpr decltype(auto) forward(T &&t) noexcept
    {
        return static_cast<decltype(t)>(t);
    }
}

#endif
