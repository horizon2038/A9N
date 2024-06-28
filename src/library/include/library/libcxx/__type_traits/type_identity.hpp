#ifndef LIBCXX_TYPE_IDENTITY_HPP
#define LIBCXX_TYPE_IDENTITY_HPP

namespace liba9n::std
{
    template<typename T>
    struct type_identity
    {
        using type = T;
    };
}

#endif
