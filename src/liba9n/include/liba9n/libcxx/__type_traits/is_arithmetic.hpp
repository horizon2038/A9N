#ifndef LIBCXX_IS_ARITHMETIC_HPP
#define LIBCXX_IS_ARITHMETIC_HPP

#include <liba9n/libcxx/__type_traits/integral_constant.hpp>
#include <liba9n/libcxx/__type_traits/is_floating_point.hpp>
#include <liba9n/libcxx/__type_traits/is_integral.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_arithmetic
        : integral_constant<bool, is_integral_v<T> || is_floating_point_v<T>>
    {
    };

    template<typename T>
    inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;
}

#endif
