#ifndef LIBCXX_REMOVE_CVREF_HPP
#define LIBCXX_REMOVE_CVREF_HPP

#include <liba9n/libcxx/__type_traits/remove_cv.hpp>
#include <liba9n/libcxx/__type_traits/remove_reference.hpp>

namespace liba9n::std
{
    template<typename T>
    struct remove_cvref
    {
        using type = remove_reference_t<remove_cv_t<T>>;
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

}

#endif
