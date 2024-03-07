#ifndef LIBCPP_TYPE_TRAITS
#define LIBCPP_TYPE_TRAITS

namespace std
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
