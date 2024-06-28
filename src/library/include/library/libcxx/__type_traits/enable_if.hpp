#ifndef LIBCXX_ENABLE_IF_HPP
#define LIBCXX_ENABLE_IF_HPP

namespace liba9n::std
{
    template<bool, typename T = void>
    struct enable_if
    {
    };

    template<typename T>
    struct enable_if<true, T>
    {
        using type = T;
    };

    template<bool B, typename T = void>
    using enable_if_t = typename enable_if<B, T>::type;
}

#endif
