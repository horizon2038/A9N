#ifndef LIBCXX_TYPE_IDENTITY_HPP
#define LIBCXX_TYPE_IDENTITY_HPP

namespace library::std
{
    template<typename T>
    struct type_identity
    {
        using type = T;
    };
}

#endif
