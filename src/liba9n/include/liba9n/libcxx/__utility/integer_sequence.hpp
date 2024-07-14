#ifndef LIBA9N_LIBCXX_UTILITY_INTEGER_SEQUENCE_HPP
#define LIBA9N_LIBCXX_UTILITY_INTEGER_SEQUENCE_HPP

#include <liba9n/libcxx/cstddef>
#include <liba9n/libcxx/__tuple/tuple_common.hpp>
#include <liba9n/libcxx/__utility/move.hpp>
#include <liba9n/libcxx/__utility/forward.hpp>
#include <liba9n/libcxx/__type_traits/is_integral.hpp>

namespace liba9n::std
{
    template<typename T, T... Ts>
        requires is_integral_v<T>
    struct integer_sequence
    {
        using value_type = T;

        constexpr auto size() noexcept
        {
            return sizeof...(Ts);
        }
    };

    template<size_t... Is>
    using index_sequence = integer_sequence<size_t, Is...>;

    // WARN:
    // portability may be compromised because __make_integer_seq is
    // a compiler extension
    template<typename T, T E>
    using make_integer_sequence = __make_integer_seq<integer_sequence, T, E>;

    template<size_t Is>
    using make_index_sequence = make_integer_sequence<size_t, Is>;

}

#endif
