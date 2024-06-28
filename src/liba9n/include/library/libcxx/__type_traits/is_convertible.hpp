#ifndef LIBCXX_IS_CONVERTIBLE_HPP
#define LIBCXX_IS_CONVERTIBLE_HPP

#include <library/libcxx/__type_traits/bool_constant.hpp>
#include <library/libcxx/__type_traits/is_void.hpp>
#include <library/libcxx/__utility/declval.hpp>

namespace liba9n::std
{
    /*
    namespace
    {
        struct dummy
        {
            void function();
        };
    }

    template<typename T, typename U, typename = void>
    struct is_convertible : false_type
    {
    };

    template<typename T, typename U>
    struct is_convertible<T, U, decltype(dummy {}.function(declval<T>()))>
        : true_type
    {
    };
    */

    // *warning*
    // portability may be compromised because __is_union is a compiler extension

    template<typename T, typename U>
    struct is_convertible : integral_constant<bool, __is_convertible_to(T, U)>
    {
    };

    template<class T, class U>
    inline constexpr bool is_convertible_v = is_convertible<T, U>::value;
}

#endif
