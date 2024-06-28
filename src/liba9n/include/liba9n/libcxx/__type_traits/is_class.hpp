#ifndef LIBCXX_IS_CLASS_HPP
#define LIBCXX_IS_CLASS_HPP

#include <liba9n/libcxx/__type_traits/integral_constant.hpp>

namespace liba9n::std
{
    // *warning*
    // portability may be compromised because __is_class is a compiler extension

    template<typename T>
    struct is_class : integral_constant<bool, __is_class(T)>
    {
    };

    template<typename T>
    inline constexpr bool is_class_v = is_class<T>::value;
}

#endif
