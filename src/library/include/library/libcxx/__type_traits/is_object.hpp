#ifndef LIBCXX_IS_OBJECT_HPP
#define LIBCXX_IS_OBJECT_HPP

#include <library/libcxx/__type_traits/integral_constant.hpp>
#include <library/libcxx/__type_traits/is_scalar.hpp>
#include <library/libcxx/__type_traits/is_array.hpp>
#include <library/libcxx/__type_traits/is_class.hpp>
#include <library/libcxx/__type_traits/is_union.hpp>

namespace library::std
{
    template<typename T>
    struct is_object
        : integral_constant<
              bool,
              is_scalar_v<T> || is_array_v<T> || is_union_v<T> || is_class_v<T>>
    {
    };

    template<typename T>
    inline constexpr bool is_object_v = is_object<T>::value;
}

#endif
