#ifndef LIBA9N_NOT_NULL_HPP
#define LIBA9N_NOT_NULL_HPP

#include "liba9n/libcxx/__type_traits/is_convertible.hpp"
#include <liba9n/libcxx/type_traits>
#include <liba9n/libcxx/utility>

namespace liba9n
{
    template<typename T>
    class not_null
    {
        // type definition
        using TElement   = liba9n::std::remove_reference_t<liba9n::std::remove_pointer_t<T>>;
        using TReference = TElement &;
        using TPointer   = TElement *;

        // friend definition
        template<typename U>
        friend class not_null;

      public:
        template<typename U = T>
            requires(liba9n::std::is_convertible_v<
                     liba9n::std::remove_reference_t<liba9n::std::remove_pointer_t<U>> *,
                     TPointer>)
        constexpr not_null(const U &reference) : pointer(liba9n::std::addressof(reference))
        {
        }

        constexpr not_null(const not_null &other) = default;
        constexpr not_null(not_null &&other)      = default;

        template<typename U>
            requires(liba9n::std::is_convertible_v<typename not_null<U>::TPointer, TPointer>)
        constexpr not_null(const not_null<U> &other) : pointer(liba9n::std::addressof(other.get()))
        {
        }

        template<typename U>
            requires(liba9n::std::is_convertible_v<typename not_null<U>::TPointer, TPointer>)
        constexpr not_null(not_null<U> &&other) : pointer(liba9n::std::addressof(other.get()))
        {
        }

        // operator section
        template<typename U = T>
            requires(liba9n::std::is_convertible_v<
                     liba9n::std::remove_reference_t<liba9n::std::remove_pointer_t<U>> *,
                     TPointer>)
        constexpr not_null &operator=(const U &reference)
        {
            pointer = liba9n::std::addressof(reference);
        }

        constexpr not_null &operator=(const not_null &other) = default;
        constexpr not_null &operator=(not_null &&other)      = default;

        template<typename U>
            requires(liba9n::std::is_convertible_v<typename not_null<U>::TPointer, TPointer>)
        constexpr not_null &operator=(const not_null<U> &other)
        {
            pointer = liba9n::std::addressof(other.get());
        }

        template<typename U>
            requires(liba9n::std::is_convertible_v<typename not_null<U>::TPointer, TPointer>)
        constexpr not_null &operator=(not_null<U> &&other)
        {
            pointer = liba9n::std::addressof(other.get());
        }

        constexpr TReference operator*() const noexcept
        {
            return *pointer;
        }

        constexpr TPointer operator->() const noexcept
        {
            return pointer;
        }

        constexpr explicit operator bool() const noexcept = delete;

        // member methods
        constexpr TReference get() const noexcept
        {
            return *pointer;
        }

      private:
        TPointer pointer;
    };
}

#endif
