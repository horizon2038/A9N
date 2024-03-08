#ifndef LIBCXX_REMOVE_REFERENCE_HPP
#define LIBCXX_REMOVE_REFERENCE_HPP

namespace library::std
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
    using remove_reference_t = typename library::std::remove_reference<T>::type;

}

#endif
