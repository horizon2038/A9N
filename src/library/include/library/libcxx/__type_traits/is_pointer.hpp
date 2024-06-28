#ifndef LIBCXX_IS_POINTER_HPP
#define LIBCXX_IS_POINTER_HPP

#include <library/libcxx/__type_traits/integral_constant.hpp>
#include <library/libcxx/__type_traits/bool_constant.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_pointer : false_type
    {
    };

    template<typename T>
    struct is_pointer<T *> : true_type
    {
    };

    template<typename T>
    struct is_pointer<const T *> : true_type
    {
    };

    template<typename T>
    struct is_pointer<volatile T *> : true_type
    {
    };

    template<typename T>
    struct is_pointer<const volatile T *> : true_type
    {
    };

    template<typename T>
    inline constexpr bool is_pointer_v = is_pointer<T>::value;
}

#endif
