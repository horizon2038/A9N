#ifndef LIBCXX_IS_CONSTRUCTIBLE_HPP
#define LIBCXX_IS_CONSTRUCTIBLE_HPP

#include <liba9n/libcxx/__type_traits/add_value_reference.hpp>
#include <liba9n/libcxx/__type_traits/integral_constant.hpp>

namespace liba9n::std
{
    // *warning*
    // portability may be compromised
    // because __is_constructible is a compiler extension

    template<typename T, typename... Args>
    struct is_constructible
        : public integral_constant<bool, __is_constructible(T, Args...)>
    {
    };

    template<typename T, typename... Args>
    inline constexpr bool is_constructible_v
        = is_constructible<T, Args...>::value;

    template<typename T>
    struct is_copy_constructible
        : public integral_constant<
              bool,
              is_constructible_v<T, add_lvalue_reference_t<const T>>>
    {
    };

    template<typename T>
    inline constexpr bool is_copy_constructible_v
        = is_copy_constructible<T>::value;

    template<typename T>
    struct is_move_constructible
        : public integral_constant<
              bool,
              is_constructible_v<T, add_rvalue_reference_t<T>>>
    {
    };

    template<typename T>
    inline constexpr bool is_move_constructible_v
        = is_move_constructible<T>::value;
}

#endif
