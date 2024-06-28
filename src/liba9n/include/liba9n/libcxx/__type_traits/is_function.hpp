#ifndef LIBCXX_IS_FUNCTION_HPP
#define LIBCXX_IS_FUNCTION_HPP

#include <liba9n/libcxx/__type_traits/bool_constant.hpp>

namespace liba9n::std
{
    template<typename>
    struct is_function : false_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...)> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...)> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) volatile> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const volatile> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const &> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile &> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile &> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const &> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) volatile &> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const volatile &> : true_type
    {
    };
    template<typename R, typename... Args>
    struct is_function<R(Args...) const &&> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile &&> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile &&> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const &&> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) volatile &&> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const volatile &&> : true_type
    {
    };

    // noexcept (after C++17)

    template<typename R, typename... Args>
    struct is_function<R(Args...) noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) volatile noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const volatile noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const & noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile & noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile & noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const & noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) volatile & noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const volatile & noexcept> : true_type
    {
    };
    template<typename R, typename... Args>
    struct is_function<R(Args...) const && noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile && noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile && noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const && noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) volatile && noexcept> : true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args..., ...) const volatile && noexcept> : true_type
    {
    };

    template<typename T>
    inline constexpr bool is_function_v = is_function<T>::value;

}

#endif
