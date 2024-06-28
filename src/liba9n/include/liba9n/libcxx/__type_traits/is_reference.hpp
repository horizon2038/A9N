#ifndef LIBCXX_IS_REFERENCE_HPP
#define LIBCXX_IS_REFERENCE_HPP

#include <liba9n/libcxx/__type_traits/integral_constant.hpp>
#include <liba9n/libcxx/__type_traits/bool_constant.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_lvalue_reference : false_type
    {
    };

    template<typename T>
    struct is_lvalue_reference<T &> : true_type
    {
    };

    template<typename T>
    struct is_rvalue_reference : false_type
    {
    };

    template<typename T>
    struct is_rvalue_reference<T &&> : true_type
    {
    };

    template<typename T>
    struct is_reference : false_type
    {
    };

    template<typename T>
    struct is_reference<T &> : true_type
    {
    };

    template<typename T>
    struct is_reference<T &&> : true_type
    {
    };

    template<typename T>
    inline constexpr bool is_reference_v = is_reference<T>::value;

    template<typename T>
    inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

    template<typename T>
    inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;
}

#endif
