#ifndef LIBCXX_DECAY_HPP
#define LIBCXX_DECAY_HPP

#include "library/libcxx/__type_traits/add_pointer.hpp"
#include "library/libcxx/__type_traits/conditional.hpp"
#include "library/libcxx/__type_traits/is_array.hpp"
#include "library/libcxx/__type_traits/is_function.hpp"
#include "library/libcxx/__type_traits/remove_extent.hpp"
#include "library/libcxx/__type_traits/remove_reference.hpp"
namespace liba9n::std
{
    template<typename T>
    struct decay
    {
      private:
        using U = remove_reference_t<T>;

      public:
        using type = typename std::conditional_t<
            is_array_v<U>,
            add_pointer_t<remove_extent_t<U>>,
            conditional_t<is_function_v<U>, U, U>>;
    };

    template<typename T>
    using decay_t = typename decay<T>::type;
}

#endif
