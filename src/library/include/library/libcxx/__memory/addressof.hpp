#ifndef LIBCXX_ADDRESSOF_HPP
#define LIBCXX_ADDRESSOF_HPP

#include <library/libcxx/__type_traits/enable_if.hpp>
#include <library/libcxx/__type_traits/is_object.hpp>

namespace library::std
{
    template<typename T>
    enable_if_t<is_object_v<T>, T *> addressof(T &arg) noexcept
    {
        return reinterpret_cast<T *>(
            &const_cast<char &>(reinterpret_cast<const volatile char &>(arg))
        );
    }

    template<typename T>
    enable_if_t<!is_object_v<T>, T *> addressof(T &arg) noexcept
    {
        return &arg;
    };
}

#endif
