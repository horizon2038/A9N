#ifndef LIBCXX_REMOVE_REFERENCE_HPP
#define LIBCXX_REMOVE_REFERENCE_HPP

namespace liba9n::std
{
    template<typename T>
    struct remove_reference
    {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T &>
    {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T &&>
    {
        using type = T;
    };

    // alias

    template<typename T>
    using remove_reference_t = typename remove_reference<T>::type;

}

#endif
