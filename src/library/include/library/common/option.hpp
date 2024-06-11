#ifndef LIBRARY_OPTION_HPP
#define LIBRARY_OPTION_HPP

#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

#include <kernel/utility/logger.hpp>

namespace library::common
{
    struct option_none_tag
    {
    };

    inline constexpr option_none_tag option_none;

    struct option_in_place_tag
    {
    };

    inline constexpr option_in_place_tag option_in_place;

    template<typename T>
    class option
    {
        template<typename>
        friend class option;

      private:
        union
        {
            char dummy;
            T value;
        };
        bool has_value_flag;

      public:
        // conditionally trivial special member functions
        constexpr option(option const &other)
            requires(library::std::is_trivially_copy_constructible_v<T>)
        = default;

        constexpr option(option &&other)
            requires(library::std::is_trivially_move_constructible_v<T>)
        = default;

        constexpr ~option()
            requires(library::std::is_trivially_destructible_v<T>)
        = default;

        constexpr option &operator=(option const &other)
            requires(library::std::is_trivially_copy_assignable_v<T>)
        = default;

        constexpr option &operator=(option &&other)
            requires(library::std::is_trivially_move_assignable_v<T>)
        = default;

        constexpr option() noexcept : dummy {}, has_value_flag(false)
        {
        }

        constexpr option([[maybe_unused]] option_none_tag n) noexcept
            : dummy {}
            , has_value_flag(false)
        {
        }

        template<typename... Args>
        constexpr option(
            [[maybe_unused]] option_in_place_tag i,
            Args... args
        ) noexcept
            : has_value_flag(true)
        {
            new (&value) T(static_cast<Args &&>(args)...);
        }

        // constructed from T : forwarding reference
        template<typename U = T>
            requires(!library::std::
                         is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        constexpr option(U &&u) noexcept
            : value(library::std::forward<U>(u))
            , has_value_flag(true)
        {
        }

        // constructed from option<T> : copy
        constexpr option(const option &other) noexcept
            : has_value_flag(other.has_value_flag)
        {
            if (!other.has_value_flag)
            {
                return;
            }

            new (&value) T(other.value);
        }

        // constructed from option<T> : move
        constexpr option(option &&other) noexcept
            : has_value_flag(other.has_value_flag)
        {
            if (!other.has_value_flag)
            {
                return;
            }

            new (&value) T(library::std::move<T>(other.value));
            other.has_value_flag = false;
        }

        // constructed from option<U> : copy
        template<typename U>
            requires(!library::std::is_reference_v<U> && library::std::is_convertible_v<T, U>)
        constexpr option(const option<U> &other) noexcept
            : has_value_flag(other.has_value_flag)
        {
            if (!other.has_value_flag)
            {
                return;
            }

            new (&value) T(static_cast<T>(other.value));
        }

        // constructed from option<U> : move
        template<typename U>
            requires(!library::std::is_reference_v<U> && library::std::is_convertible_v<T, U>)
        constexpr option(option<U> &&other) noexcept
            : has_value_flag(other.has_value_flag)
        {
            if (!other.has_value_flag)
            {
                return;
            }

            new (&value) T(static_cast<T>(library::std::move(other.value)));
            other.has_value_flag = false;
        }

        constexpr ~option() noexcept
            requires(!library::std::is_trivially_destructible_v<T>)
        {
            if (!has_value_flag)
            {
                return;
            }

            value.~T();
        }

        template<typename U>
            requires(library::std::is_convertible_v<T, U> && !library::std::is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        constexpr option &operator=(U &&u) noexcept
        {
            new (&value) T(static_cast<T>(library::std::forward<U>(u)));
            has_value_flag = true;

            return *this;
        }

        constexpr option &operator=(const option &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value_flag;
            if (other.has_value_flag)
            {
                new (&value) T(other.value);
            }

            return *this;
        }

        constexpr option &operator=(option &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value_flag;
            if (other.has_value_flag)
            {
                new (&value) T(library::std::move(other.value));
            }
            other.has_value_flag = false;

            return *this;
        }

        template<typename U>
            requires(library::std::is_convertible_v<T, U> && library::std::is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        constexpr option &operator=(const option<U> &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value_flag;
            if (other.has_value_flag)
            {
                new (&value) T(static_cast<T>(other.value));
            }

            return *this;
        }

        template<typename U>
            requires(library::std::is_convertible_v<T, U> && library::std::is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        constexpr option &operator=(option<U> &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value_flag;
            if (other.has_value_flag)
            {
                new (&value) T(static_cast<T>(library::std::move(other.value)));
            }
            other.has_value_flag = false;

            return *this;
        }

        constexpr T &operator*() &
        {
            return value;
        }

        constexpr T &operator*() const &
        {
            return value;
        }

        constexpr T &operator*() &&
        {
            return library::std::move(value);
        }

        constexpr T &operator*() const &&
        {
            return library::std::move(value);
        }

        constexpr explicit operator bool() const
        {
            return has_value_flag;
        }

        // no check is performed
        // because there is no exception mechanism

        constexpr auto &&unwrap() &
        {
            return value;
        }

        constexpr auto &&unwrap() const &
        {
            return value;
        }

        constexpr auto &&unwrap() &&
        {
            return static_cast<T>(library::std::move(value));
        }

        constexpr auto &&unwrap() const &&
        {
            return static_cast<T>(library::std::move(value));
        }

        constexpr bool has_value() const noexcept
        {
            return has_value_flag;
        }

        // TODO: add monadic operations :
        // e.g., and_then(), transform(), or_else()
    };

    // deduction guide
    template<typename T>
    option(T) -> option<T>;

    template<typename T, typename... Args>
    constexpr auto make_option_some(Args... args)
        -> option<library::std::remove_cvref_t<T>>
    {
        return option<T>(option_in_place, args...);
    }

    inline constexpr auto make_option_none() -> decltype(auto)
    {
        return option_none;
    }
}

#endif
