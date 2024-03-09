#ifndef LIBCXX_IS_ENUM_HPP
#define LIBCXX_IS_ENUM_HPP

#include <library/libcxx/__type_traits/integral_constant.hpp>

namespace library::std
{
    // *warning*
    // portability may be compromised because __is_enum is a compiler extension

    template<typename T>
    struct is_enum : integral_constant<bool, __is_enum(T)>
    {
    };

    template<typename T>
    inline constexpr bool is_enum_v = is_enum<T>::value;
}

#endif
