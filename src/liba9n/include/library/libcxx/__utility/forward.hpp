#ifndef LIBCXX_FORWARD_HPP
#define LIBCXX_FORWARD_HPP

#include <library/libcxx/__type_traits/remove_reference.hpp>

namespace liba9n::std
{
    template<typename T>
    T &&forward(remove_reference_t<T> &t) noexcept
    {
        return static_cast<T &&>(t);
    }

    template<typename T>
    T &&forward(remove_reference_t<T> &&t) noexcept
    {
        return static_cast<T &&>(t);
    }
}

#endif
