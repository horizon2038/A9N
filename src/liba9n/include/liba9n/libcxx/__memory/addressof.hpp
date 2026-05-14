#ifndef LIBCXX_ADDRESSOF_HPP
#define LIBCXX_ADDRESSOF_HPP

#include <liba9n/libcxx/__type_traits/enable_if.hpp>
#include <liba9n/libcxx/__type_traits/is_object.hpp>

namespace liba9n::std
{
    template<typename T>
        requires(is_object_v<T>)
    T *addressof(T &arg) noexcept
    {
        return reinterpret_cast<T *>(&const_cast<char &>(reinterpret_cast<const volatile char &>(arg)));
    }

    template<typename T>
        requires(!is_object_v<T>)
    T *addressof(T &arg) noexcept
    {
        return &arg;
    }

}

#endif
