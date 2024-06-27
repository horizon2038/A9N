#ifndef LIBRARY_OPTION_HPP
#define LIBRARY_OPTION_HPP

#include "library/libcxx/__type_traits/is_constructible.hpp"
#include <library/libcxx/utility>
#include <library/libcxx/type_traits>
#include <library/libcxx/functional>

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
    class option;

    template<typename T>
    concept is_option = library::std::is_same_v<
        library::std::remove_cvref_t<T>,
        option<typename library::std::remove_cvref_t<T>::some_type>>;

    template<typename T>
    class option
    {
        // TODO: value_type -> some_type
        using some_type = T;

        template<typename>
        friend class option;

      private:
        union
        {
            char dummy;
            T some_value;
        };
        bool is_some_flag;

        // helper methods
        template<typename U>
            requires library::std::is_convertible_v<U, T>
        constexpr void update_some_value(U &&new_some_value)
        {
            init_some_value();
            new (&some_value) T(library::std::forward<U>(new_some_value));
        }

        constexpr void init_some_value()
        {
            if (is_none())
            {
                return;
            }

            if constexpr (library::std::is_trivially_destructible_v<T>)
            {
                return;
            }
            else
            {
                some_value.~T();
            }
        }

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

        constexpr option() noexcept : dummy {}, is_some_flag(false)
        {
            // lazy initialization
        }

        constexpr option([[maybe_unused]] option_none_tag) noexcept
            : dummy {}
            , is_some_flag(false)
        {
            // lazy initialization
        }

        template<typename... Args>
        constexpr option(
            [[maybe_unused]] option_in_place_tag,
            Args... args
        ) noexcept
            : is_some_flag(true)
        {
            new (&some_value) T(static_cast<Args &&>(args)...);
        }

        // constructed from T : forwarding reference
        template<typename U = T>
            requires(!is_option<U>)
        constexpr option(U &&u) noexcept
            : some_value(library::std::forward<U>(u))
            , is_some_flag(true)
        {
        }

        // constructed from option<T> : copy
        constexpr option(const option &other) noexcept
            : is_some_flag(other.is_some())
        {
            if (other.is_none())
            {
                return;
            }

            new (&some_value) T(other.unwrap());
        }

        // constructed from option<T> : move
        constexpr option(option &&other) noexcept
            : is_some_flag(other.is_some())
        {
            if (other.is_none())
            {
                return;
            }

            new (&some_value) T(library::std::move<T>(other.unwrap()));
            other.is_some_flag = false;
        }

        // constructed from option<U> : copy
        template<typename U>
            requires(!library::std::is_reference_v<U> && library::std::is_convertible_v<U, T>)
        constexpr option(const option<U> &other) noexcept
            : is_some_flag(other.is_some())
        {
            if (other.is_none())
            {
                return;
            }

            new (&some_value) T(other.unwrap());
        }

        // constructed from option<U> : move
        template<typename U>
            requires(!library::std::is_reference_v<U> && library::std::is_convertible_v<U, T>)
        constexpr option(option<U> &&other) noexcept
            : is_some_flag(other.is_some())
        {
            if (other.is_none())
            {
                return;
            }

            new (&some_value) T(library::std::move(other.unwrap()));
            other.is_some_flag = false;
        }

        constexpr ~option() noexcept
            requires(!library::std::is_trivially_destructible_v<T>)
        {
            if (is_none())
            {
                return;
            }

            some_value.~T();
        }

        template<typename U>
            requires(library::std::is_convertible_v<U, T> && !is_option<U>)
        constexpr option &operator=(U &&new_some_value) noexcept
        {
            update_some_value(library::std::forward<U>(new_some_value));
            is_some_flag = true;

            return *this;
        }

        constexpr option &operator=(const option &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (other.is_none())
            {
                return;
            }

            update_some_value(other.unwrap());
            is_some_flag = true;

            return *this;
        }

        constexpr option &operator=(option &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (other.is_none())
            {
                return;
            }

            update_some_value(library::std::move(other.unwrap()));
            is_some_flag = true;

            return *this;
        }

        template<typename U>
            requires(library::std::is_convertible_v<U, T>)
        constexpr option &operator=(const option<U> &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (other.is_none())
            {
                return;
            }

            update_some_value(other.unwrap());
            is_some_flag = true;

            return *this;
        }

        template<typename U>
            requires(library::std::is_convertible_v<U, T>)
        constexpr option &operator=(option<U> &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (other.is_none())
            {
                return;
            }

            update_some_value(library::std::move(other.unwrap()));
            is_some_flag = true;

            return *this;
        }

        constexpr T &operator*() &
        {
            return some_value;
        }

        constexpr T &operator*() const &
        {
            return some_value;
        }

        constexpr T &operator*() &&
        {
            return library::std::move(some_value);
        }

        constexpr T &operator*() const &&
        {
            return library::std::move(some_value);
        }

        constexpr explicit operator bool() const
        {
            return is_some_flag;
        }

        // no check is performed
        // because there is no exception mechanism

        constexpr auto &&unwrap() &
        {
            return some_value;
        }

        constexpr auto &&unwrap() const &
        {
            return some_value;
        }

        constexpr auto &&unwrap() &&
        {
            return library::std::move(some_value);
        }

        constexpr auto &&unwrap() const &&
        {
            return library::std::move(some_value);
        }

        constexpr bool is_some() const noexcept
        {
            return is_some_flag;
        }

        constexpr bool is_none() const noexcept
        {
            return !is_some_flag;
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<T>
                  && library::std::is_convertible_v<U &&, T>
        constexpr T unwrap_or(U &&instead_some_value) &
        {
            if (is_some())
            {
                return unwrap();
            }

            return static_cast<T>(library::std::forward<U>(instead_some_value));
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<T>
                  && library::std::is_convertible_v<U, T>
        constexpr T unwrap_or(U &&instead_some_value) const &
        {
            if (is_some())
            {
                return unwrap();
            }

            return static_cast<T>(library::std::forward<U>(instead_some_value));
        }

        template<typename U>
            requires library::std::is_move_constructible_v<T>
                  && library::std::is_convertible_v<U, T>
        constexpr T unwrap_or(U &&instead_some_value) &&
        {
            if (is_some())
            {
                return library::std::move(unwrap());
            }

            return static_cast<T>(library::std::forward<U>(instead_some_value));
        }

        template<typename U>
            requires library::std::is_move_constructible_v<T>
                  && library::std::is_convertible_v<U, T>
        constexpr T unwrap_or(U &&instead_some_value) const &&
        {
            if (is_some())
            {
                return library::std::move(unwrap());
            }

            return static_cast<T>(library::std::forward<U>(instead_some_value));
        }

        // TODO: add monadic operations :
        // e.g., and_then(), transform(), or_else()

        template<
            typename Function,
            typename Tcvref = T &,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires is_option<library::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) &
        {
            if (is_none())
            {
                return library::std::remove_cvref_t<U>();
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = T const &,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires is_option<library::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) const &
        {
            if (is_none())
            {
                return library::std::remove_cvref_t<U>();
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires is_option<library::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) &&
        {
            if (is_none())
            {
                return library::std::remove_cvref_t<U>();
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            );
        }

        template<
            typename Function,
            typename Tcvref = T const &&,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires is_option<library::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) const &&
        {
            if (is_none())
            {
                return library::std::remove_cvref_t<U>();
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            );
        }

        template<
            typename Function,
            typename U = library::std::invoke_result_t<Function>>
            requires is_option<library::std::remove_cvref_t<U>>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) &
        {
            if (is_some())
            {
                return *this;
            }

            return library::std::forward<Function>(function)();
        }

        template<
            typename Function,
            typename U = library::std::invoke_result_t<Function>>
            requires is_option<library::std::remove_cvref_t<U>>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) const &
        {
            if (is_some())
            {
                return *this;
            }

            return library::std::forward<Function>(function)();
        }

        template<
            typename Function,
            typename U = library::std::invoke_result_t<Function>>
            requires is_option<library::std::remove_cvref_t<U>>
                  && library::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) &&
        {
            if (is_some())
            {
                return library::std::move(*this);
            }

            return library::std::forward<Function>(function)();
        }

        template<
            typename Function,
            typename U = library::std::invoke_result_t<Function>>
            requires is_option<library::std::remove_cvref_t<U>>
                  && library::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) const &&
        {
            if (is_some())
            {
                return library::std::move(*this);
            }

            return library::std::forward<Function>(function)();
        }

        template<
            typename Function,
            typename Tcvref = T &,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires(!library::std::is_same_v<U, option_in_place_tag> && !library::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) &
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(library::std::invoke(
                library::std::forward<Function>(function),
                unwrap()
            ));
        }

        template<
            typename Function,
            typename Tcvref = T const &,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires(!library::std::is_same_v<U, option_in_place_tag> && !library::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) const &
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(library::std::invoke(
                library::std::forward<Function>(function),
                unwrap()
            ));
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires(!library::std::is_same_v<U, option_in_place_tag> && !library::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) &&
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            ));
        }

        template<
            typename Function,
            typename Tcvref = T const &&,
            typename U = library::std::invoke_result_t<Function, Tcvref>>
            requires(!library::std::is_same_v<U, option_in_place_tag> && !library::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) const &&
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            ));
        }

        template<typename U>
        friend constexpr bool operator==(const option &lhs, option<U> rhs)
        {
            if (lhs.is_some() != rhs.is_some())
            {
                return false;
            }

            if (lhs.is_none())
            {
                return false;
            }

            return static_cast<bool>(lhs.unwrap() == rhs.unwrap());
        }

        template<typename U>
            requires(!is_option<U>)
        friend constexpr bool operator==(const option &lhs, const U &rhs)
        {
            return lhs.is_some() && static_cast<bool>(lhs.unwrap() == rhs);
        }

        friend constexpr bool operator==(const option &lhs, option_none_tag)
        {
            return lhs.is_none();
        }
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
