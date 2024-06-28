#ifndef LIBCXX_IS_NULL_POINTER_HPP
#define LIBCXX_IS_NULL_POINTER_HPP

#include <liba9n/libcxx/cstddef>
#include <liba9n/libcxx/__type_traits/is_same.hpp>
#include <liba9n/libcxx/__type_traits/remove_cv.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_null_pointer : is_same<nullptr_t, remove_cv_t<T>>
    {
    };

    template<typename T>
    inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;
}

#endif
