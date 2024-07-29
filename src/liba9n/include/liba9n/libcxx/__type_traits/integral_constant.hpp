#ifndef LIBCXX_INTEGRAL_CONSTANT_HPP
#define LIBCXX_INTEGRAL_CONSTANT_HPP

namespace liba9n::std
{
    template<typename T, T v>
    struct integral_constant
    {
        static constexpr T value = v;

        using value_type         = T;
        using type               = integral_constant<T, v>;

        constexpr operator value_type() const noexcept
        {
            return value;
        }

        constexpr value_type operator()() const noexcept
        {
            return value;
        }
    };
}

#endif
