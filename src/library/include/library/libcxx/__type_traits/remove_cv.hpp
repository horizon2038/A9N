#ifndef LIBCXX_REMOVE_CV_HPP
#define LIBCXX_REMOVE_CV_HPP

namespace library::std
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

    template<class T>
    using remove_cv_t = typename remove_cv<T>::type;

}

#endif
