#ifndef LIBCXX_IS_FUNCTION_HPP
#define LIBCXX_IS_FUNCTION_HPP

#include <library/libcxx/__type_traits/bool_constant.hpp>

namespace library::std
{
    template<typename>
    struct is_function : library::std::false_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...)> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......)> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) volatile> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const volatile> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const &> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile &> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile &> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const &> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) volatile &> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const volatile &> : library::std::true_type
    {
    };
    template<typename R, typename... Args>
    struct is_function<R(Args...) const &&> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile &&> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile &&> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const &&> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) volatile &&> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const volatile &&>
        : library::std::true_type
    {
    };

    // noexcept (after C++17)

    template<typename R, typename... Args>
    struct is_function<R(Args...) noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) volatile noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const volatile noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const & noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile & noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile & noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const & noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) volatile & noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const volatile & noexcept>
        : library::std::true_type
    {
    };
    template<typename R, typename... Args>
    struct is_function<R(Args...) const && noexcept> : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) volatile && noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args...) const volatile && noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const && noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) volatile && noexcept>
        : library::std::true_type
    {
    };

    template<typename R, typename... Args>
    struct is_function<R(Args......) const volatile && noexcept>
        : library::std::true_type
    {
    };

    template<typename T>
    inline constexpr bool is_function_v = is_function<T>::value;

}

#endif
