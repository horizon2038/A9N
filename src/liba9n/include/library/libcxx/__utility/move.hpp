#ifndef LIBCXX_MOVE
#define LIBCXX_MOVE

#include <library/libcxx/__type_traits/remove_reference.hpp>

namespace liba9n::std
{
    template<typename T>
    remove_reference_t<T> &&move(T &&t) noexcept
    {
        return static_cast<remove_reference_t<T> &&>(t);
    }
}

#endif
