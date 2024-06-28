#ifndef LIBCXX_IS_MEMBER_POINTER_HPP
#define LIBCXX_IS_MEMBER_POINTER_HPP

#include <liba9n/libcxx/__type_traits/bool_constant.hpp>
#include <liba9n/libcxx/__type_traits/remove_cv.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_member_pointer_helper : false_type
    {
    };

    template<typename T, typename U>
    struct is_member_pointer_helper<T U::*> : true_type
    {
    };

    template<typename T>
    struct is_member_pointer : is_member_pointer_helper<remove_cv_t<T>>
    {
    };

    template<typename T>
    inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

}

#endif
