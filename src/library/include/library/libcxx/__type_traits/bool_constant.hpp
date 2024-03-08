#ifndef LIBCXX_BOOL_CONSTANT_HPP
#define LIBCXX_BOOL_CONSTANT_HPP

#include <library/libcxx/__type_traits/integral_constant.hpp>

namespace library::std
{
    template<bool B>
    using bool_constant = integral_constant<bool, B>;

    using true_type = bool_constant<true>;

    using false_type = bool_constant<false>;

}

#endif
