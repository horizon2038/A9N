#ifndef LIBA9N_OPTON_COMMON_HPP
#define LIBA9N_OPTON_COMMON_HPP

#include <liba9n/libcxx/type_traits>

namespace liba9n
{
    struct option_in_place_tag
    {
    };

    inline constexpr option_in_place_tag option_in_place;

    struct option_none_tag
    {
    };

    inline constexpr option_none_tag option_none;

    template<typename T>
    class option;

    template<typename T>
    concept is_option = liba9n::std::is_same_v<
        liba9n::std::remove_cvref_t<T>,
        option<typename liba9n::std::remove_cvref_t<T>::some_type>>;

    // deduction guide
    template<typename T>
    option(T) -> option<T>;

    template<typename T, typename... Args>
    constexpr auto make_option_some(Args... args)
        -> option<liba9n::std::remove_cvref_t<T>>
    {
        return option<T>(option_in_place, args...);
    }

    inline constexpr auto make_option_none() -> decltype(auto)
    {
        return option_none;
    }
}

#endif
