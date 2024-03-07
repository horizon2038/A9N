#ifndef LIBCXX_MOVE
#define LIBCXX_MOVE

#include <library/libcxx/__type_traits/remove_reference.hpp>

namespace std
{
    template<typename T>
    typename std::remove_reference<T>::type &&move(T &&t) noexcept
    {
        return static_cast<typename std::remove_reference<T>::type &&>(t);
    }
}

#endif
