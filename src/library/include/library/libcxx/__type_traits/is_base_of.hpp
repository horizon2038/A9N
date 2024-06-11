#ifndef LIBCXX_IS_BASE_OF_HPP
#define LIBCXX_IS_BASE_OF_HPP

#include "library/libcxx/__type_traits/integral_constant.hpp"
namespace library::std
{
    // *warning*
    // portability may be compromised because __is_union is a compiler extension

    template<typename T, typename U>
    struct is_base_of : integral_constant<bool, __is_base_of(T, U)>
    {
    };

    template<typename T, typename U>
    inline constexpr bool is_base_of_v = is_base_of<T, U>::value;
}

#endif
