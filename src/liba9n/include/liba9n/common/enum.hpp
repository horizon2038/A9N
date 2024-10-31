#ifndef LIBA9N_ENUM_HPP
#define LIBA9N_ENUM_HPP

#include <liba9n/libcxx/type_traits>

namespace liba9n
{
    template<typename Enum, typename EnumBase = liba9n::std::underlying_type_t<Enum>>
        requires(liba9n::std::is_enum_v<Enum>)
    EnumBase enum_cast(Enum e)
    {
        return static_cast<EnumBase>(e);
    }
}

#endif
