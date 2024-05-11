#ifndef LIBCXX_IS_VOID_HPP
#define LIBCXX_IS_VOID_HPP

#include <library/libcxx/__type_traits/is_same.hpp>
#include <library/libcxx/__type_traits/remove_cv.hpp>

namespace library::std
{
    template<typename T>
    struct is_void : is_same<void, remove_cv_t<T>>
    {
    };

    template<typename T>
    inline constexpr bool is_void_v = is_void<T>::value;

}

#endif
