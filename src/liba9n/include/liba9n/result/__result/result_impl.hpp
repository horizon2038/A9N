#ifndef LIBA9N_RESULT_IMPL_HPP
#define LIBA9N_RESULT_IMPL_HPP

#include <liba9n/result/__result/result_common.hpp>

#include <liba9n/libcxx/functional>
#include <liba9n/libcxx/utility>
#include <liba9n/libcxx/type_traits>

namespace liba9n
{
    template<typename T, typename E>
        requires(!liba9n::std::is_same_v<T, E>)
    class result
    {
        using ok_type    = T;
        using error_type = E;

        // if you using specify friend with concepts,
        // should declared template parameters
        // i.e., <U>, <F>
        template<typename U, typename F>
            requires(!liba9n::std::is_same_v<U, F>)
        friend class result;

      private:
        union
        {
            // NOTE:
            // hypothesis : since it should be guaranteed to have `T` or `E`,
            // the dummy may not be necessary ?
            // no need for lazy initialization like option<T> ?
            char dummy;
            T    ok_value;
            E    error_value;
        };
        bool is_ok_flag;

        // helper methods

        template<typename U>
            requires liba9n::std::is_convertible_v<U, T>
        constexpr void update_ok_value(U &&new_ok_value)
        {
            init_ok_value();
            new (&error_value) T(liba9n::std::forward<U>(new_ok_value));
        }

        constexpr void init_ok_value()
        {
            if (is_error())
            {
                return;
            }

            if constexpr (liba9n::std::is_trivially_destructible_v<T>)
            {
                return;
            }
            else
            {
                ok_value.~T();
            }
        }

        template<typename F>
            requires liba9n::std::is_convertible_v<F, E>
        constexpr void update_error_value(F &&new_error_value)
        {
            init_error_value();
            new (&error_value) E(liba9n::std::forward<F>(new_error_value));
        }

        constexpr void init_error_value()
        {
            // is_ok()
            if (is_ok())
            {
                return;
            }

            if constexpr (liba9n::std::is_trivially_destructible_v<E>)
            {
                return;
            }
            else
            {
                error_value.~E();
            }
        }

      public:
        // conditionally trivial special member functions
        constexpr result(const result &other)
            requires liba9n::std::is_trivially_copy_constructible_v<T>
                      && liba9n::std::is_trivially_copy_constructible_v<E>
        = default;

        constexpr result(result &&other)
            requires liba9n::std::is_trivially_move_constructible_v<T>
                      && liba9n::std::is_trivially_move_constructible_v<E>
        = default;

        constexpr ~result()
            requires liba9n::std::is_trivially_destructible_v<T>
                      && liba9n::std::is_trivially_destructible_v<E>
        = default;

        constexpr result &operator=(result const &other)
            requires liba9n::std::is_trivially_copy_assignable_v<T>
                      && liba9n::std::is_trivially_copy_assignable_v<E>
        = default;

        constexpr result &operator=(result &&other)
            requires liba9n::std::is_trivially_move_assignable_v<T>
                      && liba9n::std::is_trivially_move_assignable_v<E>
        = default;

        // default constructor (deleted)
        // the result must always hold a value of `T` or `E`.
        constexpr result() noexcept = delete;

        template<typename... Args>
        constexpr result(
            [[maybe_unused]] result_in_place_tag,
            [[maybe_unused]] result_ok_tag,
            Args... args
        ) noexcept
            : ok_value(std::forward<Args>(args)...)
            , is_ok_flag { true }
        {
            new (&ok_value) T(static_cast<Args &&>(args)...);
        }

        template<typename... Args>
        constexpr result(
            [[maybe_unused]] result_in_place_tag,
            [[maybe_unused]] result_error_tag,
            Args... args
        ) noexcept
            : error_value(std::forward<Args>(args)...)
            , is_ok_flag { false }
        {
            new (&error_value) E(static_cast<Args &&>(args)...);
        }

        // deduction constructor
        template<typename U>
            requires(!is_result<U> && liba9n::std::is_convertible_v<liba9n::std::remove_cvref_t<U>, T>)
        constexpr result(U &&new_ok_value) noexcept : is_ok_flag { true }
        {
            new (&ok_value) T(liba9n::std::forward<T>(new_ok_value));
        }

        template<typename F>
            requires(!is_result<F> && liba9n::std::is_convertible_v<liba9n::std::remove_cvref_t<F>, E>)
        constexpr result(F &&new_error_value) noexcept : is_ok_flag { false }
        {
            new (&error_value) E(liba9n::std::forward<E>(new_error_value));
        }

        // obvious constructors
        //  - for future extensions
        //  (changes to allow the same type for T and E).
        template<typename U>
            requires(!is_result<U> && liba9n::std::is_convertible_v<liba9n::std::remove_cvref_t<U>, T>)
        constexpr result(result_ok_tag, U &&new_ok_value) noexcept
            : is_ok_flag { true }
        {
            new (&ok_value) T(liba9n::std::forward<T>(new_ok_value));
        }

        template<typename F>
            requires(!is_result<F> && liba9n::std::is_convertible_v<liba9n::std::remove_cvref_t<F>, E>)
        constexpr result(result_error_tag, F &&new_error_value) noexcept
            : is_ok_flag(false)
        {
            new (&error_value) E(liba9n::std::forward<E>(new_error_value));
        }

        constexpr result(const result &other) noexcept
            : is_ok_flag { other.is_ok() }
        {
            if (other.is_ok())
            {
                new (&ok_value) T(other.unwrap());
            }
            else
            {
                new (&error_value) E(other.unwrap_error());
            }
        }

        constexpr result(result &&other) noexcept : is_ok_flag { other.is_ok() }
        {
            if (other.is_ok())
            {
                new (&ok_value) T(liba9n::std::move(other.unwrap()));
            }
            else
            {
                new (&error_value) E(liba9n::std::move(other.unwrap_error()));
            }

            // remove this
            other.is_ok_flag = false;
        }

        // add result<U, F> constructor

        constexpr ~result() noexcept
        {
            // constexpr-if-else ?
            if (is_ok())
            {
                if constexpr (!liba9n::std::is_trivially_destructible_v<T>)
                {
                    ok_value.~T();
                }
            }
            else
            {
                if constexpr (!liba9n::std::is_trivially_destructible_v<E>)
                {
                    error_value.~E();
                }
            }
        }

        template<typename U>
            requires(!is_result<U> && liba9n::std::is_convertible_v<U, T>)
        constexpr result &operator=(U &&new_ok_value) noexcept
        {
            if (is_error())
            {
                init_error_value();
            }

            update_ok_value((liba9n::std::forward<T>(new_ok_value)));
            is_ok_flag = true;

            return *this;
        }

        template<typename F>
            requires(!is_result<F> && liba9n::std::is_convertible_v<F, E>)
        constexpr result &operator=(F &&new_error_value) noexcept
        {
            if (is_ok())
            {
                init_ok_value();
            }

            // update
            update_error_value(liba9n::std::forward<F>(new_error_value));
            is_ok_flag = false;

            return *this;
        }

        // error assign operator

        constexpr result &operator=(const result &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (is_error() && other.is_ok())
            {
                init_error_value();
            }
            if (is_ok() && other.is_error())
            {
                init_ok_value();
            }

            if (other.is_ok())
            {
                update_ok_value(other.unwrap());
            }
            else
            {
                update_error_value(other.unwrap_error());
            }

            is_ok_flag = other.is_ok();

            return *this;
        }

        constexpr result &operator=(result &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (is_error() && other.is_ok())
            {
                init_error_value();
            }
            if (is_ok() && other.is_error())
            {
                init_ok_value();
            }

            if (other.is_ok())
            {
                update_ok_value(liba9n::std::move(other.unwrap()));
            }
            else
            {
                update_error_value(liba9n::std::move(other.unwrap_error()));
            }

            is_ok_flag = other.is_ok();

            return *this;
        }

        template<typename U, typename F>
            requires liba9n::std::is_convertible_v<U, T>
                  && liba9n::std::is_convertible_v<F, E>
        constexpr result &operator=(const result<U, F> &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (is_error() && other.is_ok())
            {
                init_error_value();
            }
            if (is_ok() && other.is_error())
            {
                init_ok_value();
            }

            if (other.is_ok())
            {
                update_ok_value(other.unwrap());
            }
            else
            {
                update_error_value(other.unwrap_error());
            }

            is_ok_flag = other.is_ok();

            return *this;
        }

        template<typename U, typename F>
            requires liba9n::std::is_convertible_v<U, T>
                  && liba9n::std::is_convertible_v<F, E>
        constexpr result &operator=(result<U, F> &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (is_error() && other.is_ok())
            {
                init_error_value();
            }
            if (is_ok() && other.is_error())
            {
                init_ok_value();
            }

            if (other.is_ok())
            {
                update_ok_value(liba9n::std::move(other.unwrap()));
            }
            else
            {
                update_error_value(liba9n::std::move(other.unwrap_error()));
            }

            is_ok_flag = other.is_ok();

            return *this;
        }

        constexpr T &operator*() &
        {
            return ok_value;
        }

        constexpr T &operator*() const &
        {
            return ok_value;
        }

        constexpr T &operator*() &&
        {
            return liba9n::std::move(ok_value);
        }

        constexpr T &operator*() const &&
        {
            return liba9n::std::move(ok_value);
        }

        constexpr auto operator->() noexcept -> T *
        {
            return liba9n::std::addressof(ok_value);
        }

        constexpr auto operator->() const noexcept -> T const *
        {
            return liba9n::std::addressof(ok_value);
        }

        constexpr explicit operator bool() const
        {
            return is_ok_flag;
        }

        constexpr auto &&unwrap() &
        {
            return ok_value;
        }

        constexpr auto &&unwrap() const &
        {
            return ok_value;
        }

        constexpr auto &&unwrap() &&
        {
            return liba9n::std::move(ok_value);
        }

        constexpr auto &&unwrap() const &&
        {
            return liba9n::std::move(ok_value);
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
            return liba9n::std::move(error_value);
        }

        constexpr auto &&unwrap_error() const &&
        {
            return liba9n::std::move(error_value);
        }

        [[nodiscard("result::is_ok() must be used.")]] constexpr bool
            is_ok() const noexcept
        {
            return is_ok_flag;
        }

        constexpr bool is_error() const noexcept
        {
            return !is_ok_flag;
        }

        template<typename U>
            requires liba9n::std::is_copy_constructible_v<U>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) &
        {
            if (is_error())
            {
                return liba9n::std::forward<U>(instead_ok_value);
            }

            return unwrap();
        }

        template<typename U>
            requires liba9n::std::is_copy_constructible_v<U>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) const &
        {
            if (is_error())
            {
                return liba9n::std::forward<U>(instead_ok_value);
            }

            return unwrap();
        }

        template<typename U>
            requires liba9n::std::is_move_constructible_v<U>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) &&
        {
            if (is_error())
            {
                return liba9n::std::forward<U>(instead_ok_value);
            }

            return liba9n::std::move(unwrap());
        }

        template<typename U>
            requires liba9n::std::is_move_constructible_v<U>
                  && liba9n::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) const &&
        {
            if (is_error())
            {
                return liba9n::std::forward<U>(instead_ok_value);
            }

            return liba9n::std::move(unwrap());
        }

        template<typename F = E>
            requires liba9n::std::is_copy_constructible_v<E>
                  && liba9n::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) &
        {
            if (is_ok())
            {
                return liba9n::std::forward<F>(instead_error_value);
            }

            return unwrap_error();
        }

        template<typename F = E>
            requires liba9n::std::is_move_constructible_v<E>
                  && liba9n::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) const &
        {
            if (is_ok())
            {
                return liba9n::std::forward<F>(instead_error_value);
            }

            return unwrap_error();
        }

        template<typename F = E>
            requires liba9n::std::is_copy_constructible_v<E>
                  && liba9n::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) &&
        {
            if (is_ok())
            {
                return liba9n::std::forward<F>(instead_error_value);
            }

            return liba9n::std::move(unwrap_error());
        }

        template<typename F = E>
            requires liba9n::std::is_move_constructible_v<E>
                  && liba9n::std::is_convertible_v<F, E>
        constexpr auto unwrap_error_or(F &&instead_error_value) const &&
        {
            if (is_ok())
            {
                return liba9n::std::forward<F>(instead_error_value);
            }

            return liba9n::std::move(unwrap_error());
        }

        // monadic operations

        // and_then(Function &&function) :
        //
        // applies a function to the contained value if it exists, and returns
        // the result.
        //
        // this method will invoke the provided callable if the `result` object
        // contains a value of type `T`, the callable must return a new `result`
        // object, allowing for chaining of (monadic) operations.
        template<
            typename Function,
            typename Tcvref = T &,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::error_type, E>
                  && std::is_copy_constructible_v<T>
        constexpr auto and_then(Function &&function) &
        {
            if (is_error())
            {
                return U(result_error, unwrap_error());
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::error_type, E>
                  && liba9n::std::is_copy_constructible_v<E>
        constexpr auto and_then(Function &&function) const &
        {
            if (is_error())
            {
                return U(result_error, unwrap_error());
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::error_type, E>
                  && liba9n::std::is_move_constructible_v<E>
        constexpr auto and_then(Function &&function) &&
        {
            if (is_error())
            {
                return U(result_error, liba9n::std::move(unwrap_error()));
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap())
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::error_type, E>
                  && liba9n::std::is_move_constructible_v<E>
        constexpr auto and_then(Function &&function) const &&
        {
            if (is_error())
            {
                return U(result_error, liba9n::std::move(unwrap_error()));
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap())
            );
        }

        // or_else(Function &&function) :
        //
        // applies a function to the contained error if it exists, and retuns
        // the result.
        //
        // this method will invoke the provided callable if the `result` object
        // contains an error of type `E`, the callable must return a new
        // `result` object, allowing for error handling or transformation.
        template<
            typename Function,
            typename Ecvref = E &,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::ok_type, T>
                  && liba9n::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) &
        {
            if (is_ok())
            {
                return U(result_ok, unwrap());
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap_error()
            );
        }

        template<
            typename Function,
            typename Ecvref = const E &,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::ok_type, T>
                  && liba9n::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) const &
        {
            if (is_ok())
            {
                return U(result_ok, unwrap());
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                unwrap_error()
            );
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::ok_type, T>
                  && liba9n::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) &&
        {
            if (is_ok())
            {
                return U(result_ok, liba9n::std::move(unwrap()));
            }

            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap_error())
            );
        }

        template<
            typename Function,
            typename Ecvref = const E &&,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && liba9n::std::is_same_v<typename U::ok_type, T>
                  && liba9n::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) const &&
        {
            if (is_ok())
            {
                return U(result_ok, liba9n::std::move(unwrap()));
            }
            return liba9n::std::invoke(
                liba9n::std::forward<Function>(function),
                liba9n::std::move(unwrap_error())
            );
        }

        // transform(Function &&function) :
        //
        // transform the contained value if it exists, by applying a
        // function to it.
        //
        // this method will invoke the provided callable if the `result` object
        // conains a value of type `T`. the callable must return a new value of
        // type `T`, allowing for transformation of the contained value while
        // keeping the type consistent.
        template<
            typename Function,
            typename Tcvref = T &,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires liba9n::std::is_copy_constructible_v<E>
        constexpr auto transform(Function &&function) &
        {
            if (is_error())
            {
                return result<U, E>(result_error, unwrap_error());
            }

            if constexpr (liba9n::std::is_void_v<U>)
            {
                liba9n::std::invoke(liba9n::std::forward<Function>(function));
                return result<void, E>();
            }
            else
            {
                return result<U, E>(
                    result_in_place,
                    result_ok,
                    liba9n::std::invoke(
                        liba9n::std::forward<Function>(function),
                        unwrap()
                    )
                );
            }
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires liba9n::std::is_copy_constructible_v<E>
        constexpr auto transform(Function &&function) const &
        {
            if (is_error())
            {
                return result<U, E>(result_error, unwrap_error());
            }

            if constexpr (liba9n::std::is_void_v<U>)
            {
                liba9n::std::invoke(liba9n::std::forward<Function>(function));
                return result<void, E>();
            }
            else
            {
                return result<U, E>(
                    result_in_place,
                    result_ok,
                    liba9n::std::invoke(
                        liba9n::std::forward<Function>(function),
                        unwrap()
                    )
                );
            }
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires liba9n::std::is_move_constructible_v<E>
        constexpr auto transform(Function &&function) &&
        {
            if (is_error())
            {
                return result<U, E>(
                    result_error,
                    liba9n::std::move(unwrap_error())
                );
            }

            if constexpr (liba9n::std::is_void_v<U>)
            {
                liba9n::std::invoke(liba9n::std::forward<Function>(function));
                return result<void, E>();
            }
            else
            {
                return result<U, E>(
                    result_in_place,
                    result_ok,
                    liba9n::std::invoke(
                        liba9n::std::forward<Function>(function),
                        liba9n::std::move(unwrap())
                    )
                );
            }
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Tcvref>>>
            requires liba9n::std::is_move_constructible_v<E>
        constexpr auto transform(Function &&function) const &&
        {
            if (is_error())
            {
                return result<U, E>(
                    result_error,
                    liba9n::std::move(unwrap_error())
                );
            }

            if constexpr (liba9n::std::is_void_v<U>)
            {
                liba9n::std::invoke(liba9n::std::forward<Function>(function));
                return result<void, E>();
            }
            else
            {
                return result<U, E>(
                    result_in_place,
                    result_ok,
                    liba9n::std::invoke(
                        liba9n::std::forward<Function>(function),
                        liba9n::std::move(unwrap())
                    )
                );
            }
        }

        template<
            typename Function,
            typename Ecvref = E &,
            typename F      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires liba9n::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) &
        {
            if (is_ok())
            {
                return result<T, F>(result_in_place, result_ok, unwrap());
            }

            return result<T, F>(
                result_error,
                liba9n::std::invoke(
                    liba9n::std::forward<Function>(function),
                    unwrap_error()
                )
            );
        }

        template<
            typename Function,
            typename Ecvref = const E &,
            typename F      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires liba9n::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) const &
        {
            if (is_ok())
            {
                return result<T, F>(result_in_place, result_ok, unwrap());
            }

            return result<T, F>(
                result_error,
                liba9n::std::invoke(
                    liba9n::std::forward<Function>(function),
                    unwrap_error()
                )
            );
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename F      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires liba9n::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) &&
        {
            if (is_ok())
            {
                return result<T, F>(
                    result_in_place,
                    result_ok,
                    liba9n::std::move(unwrap())
                );
            }

            return result<T, F>(
                result_error,
                liba9n::std::invoke(
                    liba9n::std::forward<Function>(function),
                    liba9n::std::move(unwrap_error())
                )
            );
        }

        template<
            typename Function,
            typename Ecvref = E const &&,
            typename F      = liba9n::std::remove_cvref_t<
                liba9n::std::invoke_result_t<Function, Ecvref>>>
            requires liba9n::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) const &&
        {
            if (is_ok())
            {
                return result<T, F>(
                    result_in_place,
                    result_ok,
                    liba9n::std::move(unwrap())
                );
            }

            return result<T, F>(
                result_error,
                liba9n::std::invoke(
                    liba9n::std::forward<Function>(function),
                    liba9n::std::move(unwrap_error())
                )
            );
        }

        // equality operators
        template<typename U, typename F>
            requires(!liba9n::std::is_same_v<U, F>)
        friend constexpr bool
            operator==(const result &lhs, const result<U, F> &rhs)
        {
            if (lhs.is_ok() != rhs.is_ok())
            {
                return false;
            }

            if (lhs.is_ok())
            {
                return lhs.unwrap() == rhs.unwrap();
            }
            else
            {
                return lhs.unwrap_error() == rhs.unwrap_error();
            }
        }

        template<typename U>
            requires liba9n::std::
                is_convertible_v<liba9n::std::remove_cvref_t<U>, T>
            friend constexpr bool operator==(const result &lhs, const U &rhs)
        {
            return lhs.is_ok() && static_cast<bool>(lhs.unwrap() == rhs);
        }

        template<typename F>
            requires liba9n::std::
                is_convertible_v<liba9n::std::remove_cvref_t<F>, E>
            friend constexpr bool operator==(const result &lhs, const F &rhs)
        {
            return lhs.is_error()
                && static_cast<bool>(lhs.unwrap_error() == rhs);
        }
    };
}

#endif
