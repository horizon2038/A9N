#ifndef LIBA9N_STD_UNDERLYING_TYPE_HPP
#define LIBA9N_STD_UNDERLYING_TYPE_HPP

#include <liba9n/libcxx/__type_traits/is_enum.hpp>

namespace liba9n::std
{
    namespace detail
    {
        template<typename Enum, bool = liba9n::std::is_enum_v<Enum>>
        struct underlying_type_impl;

        template<typename Enum>
        struct underlying_type_impl<Enum, false>
        {
        };

        template<typename Enum>
        struct underlying_type_impl<Enum, true>
        {
            using type = __underlying_type(Enum);
        };
    }

    template<typename Enum>
    struct underlying_type : detail::underlying_type_impl<Enum>
    {
    };

    template<typename Enum>
    using underlying_type_t = typename underlying_type<Enum>::type;

}

#endif
