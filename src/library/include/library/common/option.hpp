#ifndef LIBRARY_OPTION_HPP
#define LIBRARY_OPTION_HPP

#include "library/libcxx/__type_traits/is_trivially.hpp"
#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

#include <kernel/utility/logger.hpp>

namespace library::common
{
    struct option_none_tag
    {
    };

    inline constexpr option_none_tag option_none;

    // TODO: add constexpr support
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
        option(option const &other)
            requires(library::std::is_trivially_copy_constructible_v<T>)
        = default;

        option(option &&other)
            requires(library::std::is_trivially_move_constructible_v<T>)
        = default;

        ~option()
            requires(library::std::is_trivially_destructible_v<T>)
        = default;

        option &operator=(option const &other)
            requires(library::std::is_trivially_copy_assignable_v<T>)
        = default;

        option &operator=(option &&other)
            requires(library::std::is_trivially_move_assignable_v<T>)
        = default;

        option() : dummy {}, has_value_flag(false)
        {
        }

        option([[maybe_unused]] option_none_tag n)
            : dummy {}
            , has_value_flag(false)
        {
        }

        template<typename U = T>
            requires(!library::std::
                         is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        option(U &&u) : value(library::std::forward<U>(u))
                      , has_value_flag(true)
        {
            // constructed from T : forwarding reference
        }

        option(const option &other) : has_value_flag(other.has_value_flag)
        {
            // constructed from option<T> : copy

            if (other.has_value_flag)
            {
                new (&value) T(other.value);
            }
        }

        option(option &&other) : has_value_flag(other.has_value_flag)
        {
            // constructed from option<T> : move

            if (other.has_value_flag)
            {
                new (&value) T(library::std::move<T>(other.value));
                other.has_value_flag = false;
            }
        }

        template<typename U>
            requires(!library::std::is_reference_v<U> && library::std::is_convertible_v<T, U>)
        option(const option<U> &other) : has_value_flag(other.has_value_flag)
        {
            // constructed from option<U> : copy

            if (other.has_value_flag)
            {
                new (&value) T(static_cast<T>(other.value));
            }
        }

        template<typename U>
            requires(!library::std::is_reference_v<U> && library::std::is_convertible_v<T, U>)
        option(option<U> &&other) : has_value_flag(other.has_value_flag)
        {
            // constructed from option<U> : move

            if (other.has_value_flag)
            {
                new (&value) T(static_cast<T>(library::std::move(other.value)));
                other.has_value_flag = false;
            }
        }

        ~option()
            requires(not library::std::is_trivially_destructible_v<T>)
        {
            // init();
            if (!has_value_flag)
            {
                return;
            }
            value.~T();
        }

        template<typename U>
            requires(library::std::is_convertible_v<T, U> && !library::std::is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        option &operator=(U &&u)
        {
            new (&value) T(static_cast<T>(library::std::forward<U>(u)));
            has_value_flag = true;

            return *this;
        }

        option &operator=(const option &other)
        {
            if (this != &other)
            {
                has_value_flag = other.has_value_flag;
                if (other.has_value_flag)
                {
                    new (&value) T(other.value);
                }
            }

            return *this;
        }

        option &operator=(option &&other)
        {
            if (this != &other)
            {
                has_value_flag = other.has_value_flag;
                if (other.has_value_flag)
                {
                    new (&value) T(library::std::move(other.value));
                }

                other.has_value_flag = false;
            }

            return *this;
        }

        template<typename U>
            requires(library::std::
                         is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        option &operator=(const option<U> &other)
        {
            if (this != &other)
            {
                has_value_flag = other.has_value_flag;
                if (other.has_value_flag)
                {
                    new (&value) T(static_cast<T>(other.value));
                }
            }

            return *this;
        }

        template<typename U>
            requires(library::std::
                         is_same_v<option<T>, library::std::remove_cvref_t<U>>)
        option &operator=(option<U> &&other)
        {
            if (this != &other)
            {
                has_value_flag = other.has_value_flag;

                if (other.has_value_flag)
                {
                    new (&value)
                        T(static_cast<T>(library::std::move(other.value)));
                }

                other.has_value_flag = false;
            }

            return *this;
        }

        constexpr explicit operator bool() const
        {
            return has_value_flag;
        }

        const T &unwrap() const
        {
            return value;
        }

        T &unwrap()
        {
            return value;
        }

        bool has_value()
        {
            return has_value_flag;
        }

        // TODO: add monadic operations :
        // e.g. and_then(), transform(), or_else()
    };

    // deduction guide
    template<typename T>
    option(T) -> option<T>;

    template<typename T>
    auto make_option_some(T &&t) -> option<library::std::remove_cvref_t<T>>
    {
        return library::std::forward<T>(t);
    }

    inline auto make_option_none() -> decltype(auto)
    {
        return option_none;
    }

}

#endif
