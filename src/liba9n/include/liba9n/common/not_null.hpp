#ifndef LIBA9N_NOT_NULL_HPP
#define LIBA9N_NOT_NULL_HPP

#include <liba9n/libcxx/type_traits>
#include <liba9n/libcxx/utility>

namespace liba9n
{
    template<typename T>
    class not_null
    {
        using TElement   = liba9n::std::remove_reference_t<liba9n::std::remove_pointer_t<T>>;
        using TReference = TElement &;
        using TPointer   = TElement *;

      public:
        template<typename U>
            requires(liba9n::std::is_convertible_v<
                     TPointer,
                     liba9n::std::remove_reference_t<liba9n::std::remove_pointer_t<U>> *>)
        not_null(U &&reference)
            : pointer(liba9n::std::addressof(liba9n::std::forward<U>(reference)))
        {
        }

        template<typename U>
            requires(liba9n::std::is_convertible_v<
                     TPointer,
                     liba9n::std::remove_reference_t<liba9n::std::remove_pointer_t<U>> *>)
        not_null(const U &reference) : pointer(liba9n::std::addressof(reference))
        {
        }

        not_null(const not_null &other) = default;
        not_null(not_null &&other)      = default;

        TReference get() const noexcept
        {
            return *pointer;
        }

        TReference operator*() const noexcept
        {
            return *pointer;
        }

        TPointer operator->() const noexcept
        {
            return pointer;
        }

        explicit operator bool() const noexcept = delete;

      private:
        TPointer pointer;
    };
}

#endif
