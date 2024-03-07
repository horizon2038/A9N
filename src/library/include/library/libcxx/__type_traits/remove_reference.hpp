#ifndef LIBCXX_REMOVE_REFERENCE_HPP
#define LIBCXX_REMOVE_REFERENCE_HPP

namespace library::std
{
    template<typename T>
    struct remove_reference
    {
        typedef T type;
    };

    template<typename T>
    struct remove_reference<T &>
    {
        typedef T type;
    };

    template<typename T>
    struct remove_reference<T &&>
    {
        typedef T type;
    };
}

#endif
