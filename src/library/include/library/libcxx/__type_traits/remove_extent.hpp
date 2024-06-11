#ifndef LIBCXX_REMOVE_EXTENT_HPP
#define LIBCXX_REMOVE_EXTENT_HPP

#include <library/libcxx/cstddef>

namespace library::std
{
    template<typename T>
    struct remove_extent
    {
        using type = T;
    };

    template<typename T>
    struct remove_extent<T[]>
    {
        using type = T;
    };

    template<typename T, size_t Size>
    struct remove_extent<T[Size]>
    {
        using type = T;
    };
}

#endif
