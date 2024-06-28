#ifndef LIBCXX_IS_SAME_HPP
#define LIBCXX_IS_SAME_HPP

#include <library/libcxx/__type_traits/bool_constant.hpp>

namespace liba9n::std
{
    template<typename T, typename U>
    struct is_same : false_type
    {
    };

    template<typename T>
    struct is_same<T, T> : true_type
    {
    };

    template<typename T, typename U>
    inline constexpr bool is_same_v = is_same<T, U>::value;
}

#endif
