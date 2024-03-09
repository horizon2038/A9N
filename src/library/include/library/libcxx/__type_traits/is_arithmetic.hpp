#ifndef LIBCXX_IS_ARITHMETIC_HPP
#define LIBCXX_IS_ARITHMETIC_HPP

#include <library/libcxx/__type_traits/integral_constant.hpp>
#include <library/libcxx/__type_traits/is_integral.hpp>
#include <library/libcxx/__type_traits/is_floating_point.hpp>

namespace library::std
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
