#ifndef LIBCXX_IS_ARRAY_HPP
#define LIBCXX_IS_ARRAY_HPP

#include <library/libcxx/cstddef>
#include <library/libcxx/__type_traits/bool_constant.hpp>

namespace library::std
{
    template<typename T>
    struct is_array : false_type
    {
    };

    template<typename T>
    struct is_array<T[]> : true_type
    {
    };

    template<typename T, size_t N>
    struct is_array<T[N]> : true_type
    {
    };

    template<typename T>
    inline constexpr bool is_array_v = is_array<T>::value;
}

#endif
