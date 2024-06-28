#ifndef LIBCXX_REMOVE_POINTER_HPP
#define LIBCXX_REMOVE_POINTER_HPP

namespace liba9n::std
{
    template<typename T>
    struct remove_pointer
    {
        using type = T;
    };

    template<typename T>
    struct remove_pointer<T *>
    {
        using type = T;
    };

    template<typename T>
    struct remove_pointer<const T *>
    {
        using type = T;
    };

    template<typename T>
    struct remove_pointer<volatile T *>
    {
        using type = T;
    };

    template<typename T>
    struct remove_pointer<const volatile T *>
    {
        using type = T;
    };

    template<typename T>
    using remove_pointer_t = typename remove_pointer<T>::type;
}

#endif
