#ifndef LIBCXX_IS_FLOATING_POINT_HPP
#define LIBCXX_IS_FLOATING_POINT_HPP

#include <liba9n/libcxx/__type_traits/integral_constant.hpp>
#include <liba9n/libcxx/__type_traits/is_same.hpp>
#include <liba9n/libcxx/__type_traits/remove_cv.hpp>

namespace liba9n::std
{
    template<typename T>
    struct is_floating_point
        : integral_constant<
              bool,
              is_same_v<float, remove_cv_t<T>>
                  || is_same_v<double, remove_cv_t<T>>
                  || is_same_v<long double, remove_cv_t<T>>>
    {
    };

    template<typename T>
    inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

}

#endif
