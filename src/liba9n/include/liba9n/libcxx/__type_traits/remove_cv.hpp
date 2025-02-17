#ifndef LIBCXX_REMOVE_CV_HPP
#define LIBCXX_REMOVE_CV_HPP

namespace liba9n::std
{
    template<typename T>
    struct remove_cv
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<const T>
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<volatile T>
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<const volatile T>
    {
        using type = T;
    };

    template<typename T>
    using remove_cv_t = typename remove_cv<T>::type;

}

#endif
