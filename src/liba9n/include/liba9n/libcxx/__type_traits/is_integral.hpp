#ifndef LIBCXX_IS_INTEGRAL_HPP
#define LIBCXX_IS_INTEGRAL_HPP

#include <liba9n/libcxx/__type_traits/bool_constant.hpp>

namespace liba9n::std
{
    template<typename T>
        struct is_integral : bool_constant < requires(T t, T *p, void (*f)(T))
    {
        reinterpret_cast<T>(t);
        f(0);
        p + t;
    } > {};

    template<typename T>
    inline constexpr bool is_integral_v = is_integral<T>::value;
}

#endif
