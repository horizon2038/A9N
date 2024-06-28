#ifndef LIBCXX_IS_SCALAR_HPP
#define LIBCXX_IS_SCALAR_HPP

#include <liba9n/libcxx/__type_traits/integral_constant.hpp>

#include <liba9n/libcxx/__type_traits/is_arithmetic.hpp>
#include <liba9n/libcxx/__type_traits/is_enum.hpp>
#include <liba9n/libcxx/__type_traits/is_pointer.hpp>
#include <liba9n/libcxx/__type_traits/is_member_pointer.hpp>
#include <liba9n/libcxx/__type_traits/is_null_pointer.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_scalar
        : integral_constant<
              bool,
              is_arithmetic_v<T> || is_enum_v<T> || is_pointer_v<T>
                  || is_member_pointer_v<T> || is_null_pointer_v<T>>
    {
    };

    template<typename T>
    inline constexpr bool is_scalar_v = is_scalar<T>::value;
}

#endif
