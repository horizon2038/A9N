#ifndef LIBCXX_IS_UNION_HPP
#define LIBCXX_IS_UNION_HPP

#include <library/libcxx/__type_traits/integral_constant.hpp>

namespace liba9n::std
{
    // *warning*
    // portability may be compromised because __is_union is a compiler extension

    template<typename T>
    struct is_union : integral_constant<bool, __is_union(T)>
    {
    };

    template<typename T>
    inline constexpr bool is_union_v = is_union<T>::value;
}

#endif
