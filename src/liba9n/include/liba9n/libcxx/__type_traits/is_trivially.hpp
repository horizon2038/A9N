#ifndef LIBCXX_IS_TRIVIALLY_HPP
#define LIBCXX_IS_TRIVIALLY_HPP

#include <liba9n/libcxx/__type_traits/integral_constant.hpp>
#include <liba9n/libcxx/__type_traits/add_value_reference.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_trivially_copyable
        : integral_constant<bool, __is_trivially_copyable(T)>
    {
    };

    template<typename T>
    struct is_trivially_copy_constructible
        : integral_constant<
              bool,
              __is_trivially_constructible(T, add_lvalue_reference_t<const T>)>
    {
    };

    template<typename T>
    struct is_trivially_move_constructible
        : integral_constant<
              bool,
              __is_trivially_constructible(T, add_rvalue_reference_t<const T>)>
    {
    };

    template<typename T>
    struct is_trivially_copy_assignable
        : integral_constant<
              bool,
              __is_trivially_assignable(T, add_lvalue_reference_t<const T>)>
    {
    };

    template<typename T>
    struct is_trivially_move_assignable
        : integral_constant<
              bool,
              __is_trivially_assignable(T, add_rvalue_reference_t<const T>)>
    {
    };

#if __has_builtin(__is_trivially_destructible)
    template<typename T>
    struct is_trivially_destructible
        : integral_constant<bool, __is_trivially_destructible(T)>
    {
    };

#elif __has_builtin(__has_trivial_destructor)
    template<typename T>
    struct is_trivially_destructible
        : integral_constant<bool, __has_trivial_destructor(T)>
    {
    };

#endif

    template<typename T>
    inline constexpr bool is_trivially_copyable_v
        = is_trivially_copyable<T>::value;

    template<typename T>
    inline constexpr bool is_trivially_copy_constructible_v
        = is_trivially_copy_constructible<T>::value;

    template<typename T>
    inline constexpr bool is_trivially_move_constructible_v
        = is_trivially_move_constructible<T>::value;

    template<typename T>
    inline constexpr bool is_trivially_copy_assignable_v
        = is_trivially_copy_assignable<T>::value;

    template<typename T>
    inline constexpr bool is_trivially_move_assignable_v
        = is_trivially_move_assignable<T>::value;

    template<typename T>
    inline constexpr bool is_trivially_destructible_v
        = is_trivially_destructible<T>::value;
}

#endif
