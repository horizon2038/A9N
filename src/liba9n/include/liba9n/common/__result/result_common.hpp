#ifndef LIBRARY_COMMON_RESULT_COMMON_HPP
#define LIBRARY_COMMON_RESULT_COMMON_HPP

#include <liba9n/libcxx/type_traits>

namespace liba9n::common
{
    // tag
    struct result_in_place_tag
    {
    };

    inline constexpr result_in_place_tag result_in_place;

    struct result_ok_tag
    {
    };

    inline constexpr result_ok_tag result_ok;

    struct result_error_tag
    {
    };

    inline constexpr result_error_tag result_error;

    // result<T, E> :
    // similar to std::expected<T, E>.
    // however, to simplify type inference,
    // we require that T and E be completely different types.
    template<typename T, typename E>
        requires(!liba9n::std::is_same_v<T, E>)
    class result;

    template<typename T>
    concept is_result = liba9n::std::is_same_v<
        liba9n::std::remove_cvref_t<T>,
        result<typename T::ok_type, typename T::error_type>>;

    // deduction guide
    template<typename T, typename E>
        requires(!liba9n::std::is_same_v<T, E>)
    result(T, E) -> result<T, E>;

    template<typename T>
    result(T) -> result<T, void>;

    template<typename E>
    result(E) -> result<void, E>;

    template<typename T, typename E, typename... Args>
        requires(!liba9n::std::is_same_v<T, E>)
    constexpr result<T, E> make_result_ok(Args... args) noexcept
    {
        return result<T, E>(
            result_in_place,
            result_ok,
            static_cast<Args &&>(args)...
        );
    }

    template<typename T, typename E, typename... Args>
        requires(!liba9n::std::is_same_v<T, E>)
    constexpr result<T, E> make_result_error(Args... args) noexcept
    {
        return result<T, E>(
            result_in_place,
            result_error,
            static_cast<Args &&>(args)...
        );
    }
}

#endif
