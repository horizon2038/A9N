#ifndef LIBRARY_COMMON_RESULT_VOID_IMPL_HPP
#define LIBRARY_COMMON_RESULT_VOID_IMPL_HPP

#include <library/common/__result/result_common.hpp>

#include <library/libcxx/functional>
#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

namespace library::common
{
    // specialization for case where T is *void*.
    // this is useful for scenarios where you only want to represent an error
    // without a valid result.
    template<typename E>
        requires(!library::std::is_same_v<void, E>)
    class result<void, E>
    {
        using ok_type = void;
        using error_type = E;

        // we are friends
        template<typename U, typename F>
            requires(!library::std::is_same_v<U, F>)
        friend class result;

      private:
        union
        {
            char dummy;
            E error_value;
        };
        bool is_ok_flag;

        template<typename F>
            requires library::std::is_convertible_v<F, E>
        constexpr void update_error_value(F &&new_error_value)
        {
            init_error_value();
            new (&error_value) E(library::std::forward<F>(new_error_value));
        }

        constexpr void init_error_value()
        {
            // is_ok()
            if (is_ok())
            {
                return;
            }

            if constexpr (library::std::is_trivially_destructible_v<E>)
            {
                return;
            }

            error_value.~E();
        }

      public:
        // conditionally trivial special member functions
        // check only E
        constexpr result(const result &other)
            requires library::std::is_trivially_copy_constructible_v<E>
        = default;

        constexpr result(result &&other)
            requires library::std::is_trivially_move_constructible_v<E>
        = default;

        constexpr ~result()
            requires library::std::is_trivially_destructible_v<E>
        = default;

        constexpr result &operator=(result const &other)
            requires library::std::is_trivially_copy_assignable_v<E>
        = default;

        constexpr result &operator=(result &&other)
            requires library::std::is_trivially_move_assignable_v<E>
        = default;

        // default constructor
        // unlike result<T, E>, result<void, E> is not deleted
        // because `T` = `void` is allowed.
        constexpr result() noexcept : dummy {}, is_ok_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename... Args>
        constexpr result(
            [[maybe_unused]] result_in_place_tag,
            [[maybe_unused]] result_ok_tag
        ) noexcept
            : dummy {}
            , is_ok_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename... Args>
        constexpr result(
            [[maybe_unused]] result_in_place_tag,
            [[maybe_unused]] result_error_tag,
            Args... args
        ) noexcept
            : is_ok_flag { false }
        {
            new (&error_value) E(static_cast<Args &&>(args)...);
        }

        template<typename U = void>
            requires(!is_result<U> && library::std::is_convertible_v<U, void>)
        constexpr result(U &&new_ok_value) noexcept
            : dummy {}
            , is_ok_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(F &&new_error_value) noexcept : is_ok_flag { false }
        {
            new (&error_value) E(library::std::forward<F>(new_error_value));
        }

        // obvious constructors
        // - for future extensions
        constexpr result(result_ok_tag) noexcept : dummy {}, is_ok_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(result_error_tag, F &&new_error_value) noexcept
            : is_ok_flag { false }
        {
            new (&error_value) E(library::std::forward<F>(new_error_value));
        }

        // result<T, E> -> result<T, E>
        constexpr result(const result &other) noexcept
            : dummy {}
            , is_ok_flag { other.is_ok() }
        {
            if (other.is_ok())
            {
                return;
            }

            new (&error_value) E(other.unwrap_error());
        }

        constexpr result(result &&other) noexcept
            : dummy {}
            , is_ok_flag { other.is_ok() }
        {
            if (other.is_error())
            {
                new (&error_value) E(library::std::move(other.unwrap_error()));
            }
        }

        // result<U, F> -> result<T, E>
        template<typename U>
            requires is_result<U>
                      && library::std::
                             is_convertible_v<typename U::ok_type, void>
                      && library::std::
                             is_convertible_v<typename U::error_type, E>
        constexpr result(const U &other) noexcept
            : dummy {}
            , is_ok_flag { other.is_ok() }
        {
            if (other.is_ok())
            {
                return;
            }

            new (&error_value) E(other.unwrap_error());
        }

        template<typename U>
            requires is_result<U>
                      && library::std::
                             is_convertible_v<typename U::ok_type, void>
                      && library::std::
                             is_convertible_v<typename U::error_type, E>
        constexpr result(U &&other) noexcept
            : dummy {}
            , is_ok_flag { other.is_ok() }
        {
            if (other.is_error())
            {
                new (&error_value) E(library::std::move(other.unwrap_error()));
            }
        }

        constexpr ~result() noexcept
        {
            // `E` is not trivial
            error_value.~E();
        }

        // operators
        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<F, E>)
        constexpr result &operator=(F &&new_error_value) noexcept
        {
            update_error_value(library::std::forward<F>(new_error_value));
            is_ok_flag = false;

            return *this;
        }

        constexpr result &operator=(const result &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            is_ok_flag = other.is_ok();
            if (other.is_error())
            {
                update_error_value(other.unwrap_error());
            }

            return *this;
        }

        constexpr result &operator=(result &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            is_ok_flag = other.is_ok();
            if (other.is_error())
            {
                update_error_value(library::std::move(other.unwrap_error()));
            }

            return *this;
        }

        template<typename U, typename F>
            requires library::std::is_convertible_v<U, void>
                  && library::std::is_convertible_v<F, E>
        constexpr result &operator=(const result<U, F> &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            is_ok_flag = other.is_ok();
            if (other.is_error())
            {
                update_error_value(other.unwrap_error());
            }

            return *this;
        }

        template<typename U, typename F>
            requires library::std::is_convertible_v<U, void>
                  && library::std::is_convertible_v<F, E>
        constexpr result &operator=(result<U, F> &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            is_ok_flag = other.is_ok();
            if (other.is_error())
            {
                update_error_value(static_cast<E>(other.unwrap_error()));
            }

            return *this;
        }

        constexpr void operator*() noexcept
        {
            return;
        }

        constexpr explicit operator bool() const
        {
            return is_ok_flag;
        }

        constexpr void unwrap() &
        {
            return;
        }

        constexpr void unwrap() const &
        {
            return;
        }

        constexpr void unwrap() &&
        {
            return;
        }

        constexpr void unwrap() const &&
        {
            return;
        }

        constexpr auto &&unwrap_error() &
        {
            return error_value;
        }

        constexpr auto &&unwrap_error() const &
        {
            return error_value;
        }

        constexpr auto &&unwrap_error() &&
        {
            return library::std::move(error_value);
        }

        constexpr auto &&unwrap_error() const &&
        {
            return library::std::move(error_value);
        }

        [[nodiscard("result::is_ok() must be used.")]] constexpr bool
            is_ok() const noexcept
        {
            return is_ok_flag;
        }

        [[nodiscard("result::is_error() must be used.")]] constexpr bool
            is_error() const noexcept
        {
            return !is_ok_flag;
        }

        template<typename F = E>
            requires library::std::is_copy_constructible_v<F>
                  && library::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) &
        {
            if (is_ok())
            {
                return library::std::forward<F>(instead_error_value);
            }

            return unwrap_error();
        }

        template<typename F = E>
            requires library::std::is_copy_constructible_v<F>
                  && library::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) const &
        {
            if (is_ok())
            {
                return library::std::forward<F>(instead_error_value);
            }

            return unwrap_error();
        }

        template<typename F = E>
            requires library::std::is_move_constructible_v<F>
                  && library::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) &&
        {
            if (is_ok())
            {
                return library::std::forward<F>(instead_error_value);
            }

            return library::std::move(unwrap_error());
        }

        template<typename F = E>
            requires library::std::is_move_constructible_v<F>
                  && library::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) const &&
        {
            if (is_ok())
            {
                return library::std::forward<F>(instead_error_value);
            }

            return library::std::move(unwrap_error());
        }

        // monadic operations

        // and_then(Function &&function) :
        //
        // applies a function to the contained value if it exists, and returns
        // the result.
        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && std::is_copy_constructible_v<E>
        constexpr auto and_then(Function &&function) &
        {
            if (is_error())
            {
                return U(result_error, unwrap_error());
            }

            return library::std::invoke(library::std::forward<Function>(function
            ));
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && std::is_copy_constructible_v<E>
        constexpr auto and_then(Function &&function) const &
        {
            if (is_error())
            {
                return U(result_error, unwrap_error());
            }

            return library::std::invoke(library::std::forward<Function>(function
            ));
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && std::is_move_constructible_v<E>
        constexpr auto and_then(Function &&function) &&
        {
            if (is_error())
            {
                return U(result_error, library::std::move(unwrap_error()));
            }

            return library::std::invoke(library::std::forward<Function>(function
            ));
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && std::is_move_constructible_v<E>
        constexpr auto and_then(Function &&function) const &&
        {
            if (is_error())
            {
                return U(result_error, library::std::move(unwrap_error()));
            }

            return library::std::invoke(library::std::forward<Function>(function
            ));
        }

        template<
            typename Function,
            typename Ecvref = E &,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
        constexpr auto or_else(Function &&function) &
        {
            if (is_ok())
            {
                // uniform initialization
                return F {};
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            );
        }

        template<
            typename Function,
            typename Ecvref = E const &,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
        constexpr auto or_else(Function &&function) const &
        {
            if (is_ok())
            {
                return F {};
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            );
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
        constexpr auto or_else(Function &&function) &&
        {
            if (is_ok())
            {
                return F {};
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            );
        }

        template<
            typename Function,
            typename Ecvref = E const &&,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
        constexpr auto or_else(Function &&function) const &&
        {
            if (is_ok())
            {
                return F {};
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            );
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_copy_constructible_v<E>
        constexpr auto transform(Function &&function) &
        {
            if (is_error())
            {
                return result<U, E>(result_error, unwrap_error());
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(library::std::forward<Function>(function))
            );
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_copy_constructible_v<E>
        constexpr auto transform(Function &&function) const &
        {
            if (is_error())
            {
                return result<U, E>(result_error, unwrap_error());
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(library::std::forward<Function>(function))
            );
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_move_constructible_v<E>
        constexpr auto transform(Function &&function) &&
        {
            if (is_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(library::std::forward<Function>(function))
            );
        }

        template<
            typename Function,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_move_constructible_v<E>
        constexpr auto transform(Function &&function) const &&
        {
            if (is_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(library::std::forward<Function>(function))
            );
        }

        template<
            typename Function,
            typename Ecvref = E &,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_copy_constructible_v<E>
        constexpr auto transform_error(Function &&function) &
        {
            if (is_ok())
            {
                return result<void, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function)
                    )
                );
            }

            return result<void, E>(result_error, unwrap_error());
        }

        template<
            typename Function,
            typename Ecvref = E const &,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_copy_constructible_v<E>
        constexpr auto transform_error(Function &&function) const &
        {
            if (is_ok())
            {
                return result<void, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function)
                    )
                );
            }

            return result<void, E>(result_error, unwrap_error());
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_move_constructible_v<E>
        constexpr auto transform_error(Function &&function) &&
        {
            if (is_ok())
            {
                return result<void, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function)
                    )
                );
            }

            return result<void, E>(
                result_error,
                library::std::move(unwrap_error())
            );
        }

        template<
            typename Function,
            typename Ecvref = E const &&,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function>>>
            requires library::std::is_move_constructible_v<E>
        constexpr auto transform_error(Function &&function) const &&
        {
            if (is_ok())
            {
                return result<void, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function)
                    )
                );
            }

            return result<void, E>(
                result_error,
                library::std::move(unwrap_error())
            );
        }

        // equality operators
        template<typename F>
            requires(!library::std::is_same_v<void, F>)
        friend constexpr bool
            operator==(const result &lhs, const result<void, F> &rhs)
        {
            if (lhs.is_ok() != rhs.is_ok())
            {
                return false;
            }

            return lhs.is_ok()
                || static_cast<bool>(lhs.unwrap_error() == rhs.unwrap_error());
        }

        template<typename F>
            requires library::std::
                is_convertible_v<library::std::remove_cvref_t<F>, E>
            friend constexpr bool operator==(const result &lhs, const F &rhs)
        {
            return lhs.is_error()
                && static_cast<bool>(lhs.unwrap_error() == rhs);
        }
    };
}

#endif
