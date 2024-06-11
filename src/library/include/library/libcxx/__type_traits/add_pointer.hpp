#ifndef LIBCXX_ADD_POINTER_HPP
#define LIBCXX_ADD_POINTER_HPP

#include <library/libcxx/__type_traits/remove_reference.hpp>
#include <library/libcxx/__type_traits/type_identity.hpp>

namespace library::std
{
    namespace
    {
        template<typename T>
        auto try_add_pointer(int) -> type_identity<remove_reference_t<T> *>;

        template<typename T>
        auto try_add_pointer(...) -> type_identity<T>;
    }

    template<typename T>
    struct add_pointer : decltype(try_add_pointer<T>(0))
    {
    };

    template<typename T>
    using add_pointer_t = typename add_pointer<T>::type;
}

#endif
