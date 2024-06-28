#ifndef LIBCXX_DECLVAL_HPP
#define LIBCXX_DECLVAL_HPP

#include <library/libcxx/__type_traits/add_value_reference.hpp>

namespace liba9n::std
{
    template<typename T>
    add_rvalue_reference_t<T> declval() noexcept;
}

#endif
