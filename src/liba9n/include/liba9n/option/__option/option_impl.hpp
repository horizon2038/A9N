#ifndef LIBA9N_OPTION_IMPL_HPP
#define LIBA9N_OPTION_IMPL_HPP

#include <liba9n/option/__option/option_common.hpp>

#include <liba9n/libcxx/functional>
#include <liba9n/libcxx/type_traits>
#include <liba9n/libcxx/utility>

namespace liba9n
{
    template<typename T>
    class option
    {
        using some_type = T;

        template<typename>
        friend class option;

      private:
        union
        {
            char dummy;
            T    some_value;
        };

        bool is_some_flag;

        // helper methods
        template<typename U>
            requires liba9n::std::is_convertible_v<U, T>
        constexpr void update_some_value(U &&new_some_value)
        {
            init_some_value();
            new (&some_value) T(liba9n::std::forward<U>(new_some_value));
        }

        constexpr void init_some_value()
        {
            if (is_none())
            {
                return;
            }

            if constexpr (liba9n::std::is_trivially_destructible_v<T>)
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
        constexpr option(const option &other)
            requires(liba9n::std::is_trivially_copy_constructible_v<T>)
        = default;

        constexpr option(option &&other)
            requires(liba9n::std::is_trivially_move_constructible_v<T>)
        = default;

        constexpr ~option()
            requires(liba9n::std::is_trivially_destructible_v<T>)
        = default;

        constexpr option &operator=(const option &other)
            requires(liba9n::std::is_trivially_copy_assignable_v<T>)
        = default;

        constexpr option &operator=(option &&other)
            requires(liba9n::std::is_trivially_move_assignable_v<T>)
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
        constexpr option([[maybe_unused]] option_in_place_tag, Args... args) noexcept
            : is_some_flag(true)
        {
            new (&some_value) T(static_cast<Args &&>(args)...);
        }

        // constructed from T : forwarding reference
        template<typename U = T>
            requires(!is_option<U>)
        constexpr option(U &&u) noexcept
            : some_value(liba9n::std::forward<U>(u))
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

            new (&some_value) T(liba9n::std::move<T>(other.unwrap()));
            other.is_some_flag = false;
        }

        // constructed from option<U> : copy
        template<typename U>
            requires(!liba9n::std::is_reference_v<U> && liba9n::std::is_convertible_v<U, T>)
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
            requires(!liba9n::std::is_reference_v<U> && liba9n::std::is_convertible_v<U, T>)
        constexpr option(option<U> &&other) noexcept
            : is_some_flag(other.is_some())
        {
            if (other.is_none())
            {
                return;
            }

            new (&some_value) T(liba9n::std::move(other.unwrap()));
            other.is_some_flag = false;
        }

        constexpr ~option() noexcept
            requires(!liba9n::std::is_trivially_destructible_v<T>)
        {
            if (is_none())
            {
                return;
            }

            some_value.~T();
        }

        template<typename U>
            requires(liba9n::std::is_convertible_v<U, T> && !is_option<U>)
        constexpr option &operator=(U &&new_some_value) noexcept
        {
            update_some_value(liba9n::std::forward<U>(new_some_value));
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

            update_some_value(liba9n::std::move(other.unwrap()));
            is_some_flag = true;

            return *this;
        }

        template<typename U>
            requires(liba9n::std::is_convertible_v<U, T>)
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
            requires(liba9n::std::is_convertible_v<U, T>)
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

            update_some_value(liba9n::std::move(other.unwrap()));
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
            return liba9n::std::move(some_value);
        }

        constexpr T &operator*() const &&
        {
            return liba9n::std::move(some_value);
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
            return liba9n::std::move(some_value);
        }

        constexpr auto &&unwrap() const &&
        {
            return liba9n::std::move(some_value);
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
            requires liba9n::std::is_copy_constructible_v<T>
                  && liba9n::std::is_convertible_v<U &&, T>
        constexpr T unwrap_or(U &&instead_some_value) &
        {
            if (is_some())
            {
                return unwrap();
            }

            return static_cast<T>(liba9n::std::forward<U>(instead_some_value));
        }

        template<typename U>
            requires liba9n::std::is_copy_constructible_v<T>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr T unwrap_or(U &&instead_some_value) const &
        {
            if (is_some())
            {
                return unwrap();
            }

            return static_cast<T>(liba9n::std::forward<U>(instead_some_value));
        }

        template<typename U>
            requires liba9n::std::is_move_constructible_v<T>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr T unwrap_or(U &&instead_some_value) &&
        {
            if (is_some())
            {
                return liba9n::std::move(unwrap());
            }

            return static_cast<T>(liba9n::std::forward<U>(instead_some_value));
        }

        template<typename U>
            requires liba9n::std::is_move_constructible_v<T>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr T unwrap_or(U &&instead_some_value) const &&
        {
            if (is_some())
            {
                return liba9n::std::move(unwrap());
            }

            return static_cast<T>(liba9n::std::forward<U>(instead_some_value));
        }

        template<
            typename Function,
            typename Tcvref = T &,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) &
        {
            if (is_none())
            {
                return liba9n::std::remove_cvref_t<U>();
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) const &
        {
            if (is_none())
            {
                return liba9n::std::remove_cvref_t<U>();
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) &&
        {
            if (is_none())
            {
                return liba9n::std::remove_cvref_t<U>();
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap())
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
        constexpr auto and_then(Function &&function) const &&
        {
            if (is_none())
            {
                return liba9n::std::remove_cvref_t<U>();
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap())
            );
        }

        template<typename Function, typename U = liba9n::std::invoke_result_t<Function>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
                  && liba9n::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) &
        {
            if (is_some())
            {
                return *this;
            }

            return liba9n::std::forward<Function>(function)();
        }

        template<typename Function, typename U = liba9n::std::invoke_result_t<Function>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
                  && liba9n::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) const &
        {
            if (is_some())
            {
                return *this;
            }

            return liba9n::std::forward<Function>(function)();
        }

        template<typename Function, typename U = liba9n::std::invoke_result_t<Function>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
                  && liba9n::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) &&
        {
            if (is_some())
            {
                return liba9n::std::move(*this);
            }

            return liba9n::std::forward<Function>(function)();
        }

        template<typename Function, typename U = liba9n::std::invoke_result_t<Function>>
            requires is_option<liba9n::std::remove_cvref_t<U>>
                  && liba9n::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) const &&
        {
            if (is_some())
            {
                return liba9n::std::move(*this);
            }

            return liba9n::std::forward<Function>(function)();
        }

        template<
            typename Function,
            typename Tcvref = T &,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires(!liba9n::std::is_same_v<U, option_in_place_tag> && !liba9n::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) &
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap()
            ));
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires(!liba9n::std::is_same_v<U, option_in_place_tag> && !liba9n::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) const &
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap()
            ));
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires(!liba9n::std::is_same_v<U, option_in_place_tag> && !liba9n::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) &&
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap())
            ));
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U      = liba9n::std::invoke_result_t<Function, Tcvref>>
            requires(!liba9n::std::is_same_v<U, option_in_place_tag> && !liba9n::std::is_same_v<U, option_none_tag>)
        constexpr auto transform(Function &&function) const &&
        {
            if (is_none())
            {
                return option<U>();
            }

            return option<U>(liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap())
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

}

#endif
