#ifndef LIBCXX_ADD_VALUE_REFERENCE_HPP
#define LIBCXX_ADD_VALUE_REFERENCE_HPP

#include <liba9n/libcxx/__type_traits/type_identity.hpp>

namespace liba9n::std
{
    namespace
    {
        template<typename T>
        auto try_add_lvalue_reference(int) -> type_identity<T &>;

        template<typename T>
        auto try_add_lvalue_reference(...) -> type_identity<T>;

        template<typename T>
        auto try_add_rvalue_reference(int) -> type_identity<T &&>;

        template<typename T>
        auto try_add_rvalue_reference(...) -> type_identity<T>;

    }

    template<typename T>
    struct add_lvalue_reference : decltype(try_add_lvalue_reference<T>(0))
    {
    };

    template<typename T>
    struct add_rvalue_reference : decltype(try_add_rvalue_reference<T>(0))
    {
    };

    template<typename T>
    using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

    template<typename T>
    using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
}

#endif
