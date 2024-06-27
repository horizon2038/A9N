#ifndef LIBRARY_COMMON_RESULT_IMPL_HPP
#define LIBRARY_COMMON_RESULT_IMPL_HPP

#include <library/common/__result/result_common.hpp>

#include <library/libcxx/functional>
#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

namespace library::common
{
    template<typename T, typename E>
        requires(!library::std::is_same_v<T, E>)
    class result
    {
        using ok_type = T;
        using error_type = E;

        // if you using specify friend with concepts,
        // should declared template parameters
        // i.e., <U>, <F>
        template<typename U, typename F>
            requires(!library::std::is_same_v<U, F>)
        friend class result;

      private:
        union
        {
            // NOTE:
            // hypothesis : since it should be guaranteed to have `T` or `E`,
            // the dummy may not be necessary ?
            // no need for lazy initialization like option<T> ?
            char dummy;
            T ok_value;
            E error_value;
        };
        bool is_ok_flag;

        // helper methods

        template<typename U>
            requires library::std::is_convertible_v<U, T>
        constexpr void update_ok_value(U &&new_ok_value)
        {
            init_ok_value();
            new (&error_value) T(library::std::forward<U>(new_ok_value));
        }

        constexpr void init_ok_value()
        {
            if (is_error())
            {
                return;
            }

            if constexpr (library::std::is_trivially_destructible_v<T>)
            {
                return;
            }

            ok_value.~T();
        }

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
        constexpr result(const result &other)
            requires library::std::is_trivially_copy_constructible_v<T>
                      && library::std::is_trivially_copy_constructible_v<E>
        = default;

        constexpr result(result &&other)
            requires library::std::is_trivially_move_constructible_v<T>
                      && library::std::is_trivially_move_constructible_v<E>
        = default;

        constexpr ~result()
            requires library::std::is_trivially_destructible_v<T>
                      && library::std::is_trivially_destructible_v<E>
        = default;

        constexpr result &operator=(result const &other)
            requires library::std::is_trivially_copy_assignable_v<T>
                      && library::std::is_trivially_copy_assignable_v<E>
        = default;

        constexpr result &operator=(result &&other)
            requires library::std::is_trivially_move_assignable_v<T>
                      && library::std::is_trivially_move_assignable_v<E>
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
            requires(!is_result<U> && library::std::is_convertible_v<library::std::remove_cvref_t<U>, T>)
        constexpr result(U &&new_ok_value) noexcept : is_ok_flag { true }
        {
            new (&ok_value) T(library::std::forward<T>(new_ok_value));
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(F &&new_error_value) noexcept : is_ok_flag { false }
        {
            new (&error_value) E(library::std::forward<E>(new_error_value));
        }

        // obvious constructors
        //  - for future extensions
        //  (changes to allow the same type for T and E).
        template<typename U>
            requires(!is_result<U> && library::std::is_convertible_v<library::std::remove_cvref_t<U>, T>)
        constexpr result(result_ok_tag, U &&new_ok_value) noexcept
            : is_ok_flag { true }
        {
            new (&ok_value) T(library::std::forward<T>(new_ok_value));
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(result_error_tag, F &&new_error_value) noexcept
            : is_ok_flag(false)
        {
            new (&error_value) E(library::std::forward<E>(new_error_value));
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
                new (&ok_value) T(library::std::move(other.unwrap()));
            }
            else
            {
                new (&error_value) E(library::std::move(other.unwrap_error()));
            }

            // remove this
            other.is_ok_flag = false;
        }

        // add result<U, F> constructor

        constexpr ~result() noexcept
        {
            if (is_ok())
            {
                if constexpr (!library::std::is_trivially_destructible_v<T>)
                {
                    ok_value.~T();
                }
            }
            else
            {
                if constexpr (!library::std::is_trivially_destructible_v<E>)
                {
                    error_value.~E();
                }
            }
        }

        template<typename U>
            requires(!is_result<U> && library::std::is_convertible_v<U, T>)
        constexpr result &operator=(U &&new_ok_value) noexcept
        {
            if (is_error())
            {
                init_error_value();
            }

            update_ok_value((library::std::forward<T>(new_ok_value)));
            is_ok_flag = true;

            return *this;
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<F, E>)
        constexpr result &operator=(F &&new_error_value) noexcept
        {
            if (is_ok())
            {
                init_ok_value();
            }

            // update
            update_error_value(library::std::forward<F>(new_error_value));
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
                update_ok_value(library::std::move(other.unwrap()));
            }
            else
            {
                update_error_value(library::std::move(other.unwrap_error()));
            }

            is_ok_flag = other.is_ok();

            return *this;
        }

        template<typename U, typename F>
            requires library::std::is_convertible_v<U, T>
                  && library::std::is_convertible_v<F, E>
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
            requires library::std::is_convertible_v<U, T>
                  && library::std::is_convertible_v<F, E>
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
                update_ok_value(library::std::move(other.unwrap()));
            }
            else
            {
                update_error_value(library::std::move(other.unwrap_error()));
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
            return library::std::move(ok_value);
        }

        constexpr T &operator*() const &&
        {
            return library::std::move(ok_value);
        }

        constexpr auto operator->() noexcept -> T *
        {
            return library::std::addressof(ok_value);
        }

        constexpr auto operator->() const noexcept -> T const *
        {
            return library::std::addressof(ok_value);
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
            return library::std::move(ok_value);
        }

        constexpr auto &&unwrap() const &&
        {
            return library::std::move(ok_value);
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

        constexpr bool is_error() const noexcept
        {
            return !is_ok_flag;
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) &
        {
            if (is_error())
            {
                return library::std::forward<U>(instead_ok_value);
            }

            return unwrap();
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) const &
        {
            if (is_error())
            {
                return library::std::forward<U>(instead_ok_value);
            }

            return unwrap();
        }

        template<typename U>
            requires library::std::is_move_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) &&
        {
            if (is_error())
            {
                return library::std::forward<U>(instead_ok_value);
            }

            return library::std::move(unwrap());
        }

        template<typename U>
            requires library::std::is_move_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&instead_ok_value) const &&
        {
            if (is_error())
            {
                return library::std::forward<U>(instead_ok_value);
            }

            return library::std::move(unwrap());
        }

        template<typename F = E>
            requires library::std::is_copy_constructible_v<E>
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
            requires library::std::is_move_constructible_v<E>
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
            requires library::std::is_copy_constructible_v<E>
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
            requires library::std::is_move_constructible_v<E>
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
        //
        // this method will invoke the provided callable if the `result` object
        // contains a value of type `T`, the callable must return a new `result`
        // object, allowing for chaining of (monadic) operations.
        template<
            typename Function,
            typename Tcvref = T &,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && std::is_copy_constructible_v<T>
        constexpr auto and_then(Function &&function) &
        {
            if (is_error())
            {
                return U(result_error, unwrap_error());
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && library::std::is_copy_constructible_v<E>
        constexpr auto and_then(Function &&function) const &
        {
            if (is_error())
            {
                return U(result_error, unwrap_error());
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap()
            );
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && library::std::is_move_constructible_v<E>
        constexpr auto and_then(Function &&function) &&
        {
            if (is_error())
            {
                return U(result_error, library::std::move(unwrap_error()));
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::error_type, E>
                  && library::std::is_move_constructible_v<E>
        constexpr auto and_then(Function &&function) const &&
        {
            if (is_error())
            {
                return U(result_error, library::std::move(unwrap_error()));
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
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
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::ok_type, T>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) &
        {
            if (is_ok())
            {
                return U(result_ok, unwrap());
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            );
        }

        template<
            typename Function,
            typename Ecvref = const E &,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::ok_type, T>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto or_else(Function &&function) const &
        {
            if (is_ok())
            {
                return U(result_ok, unwrap());
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            );
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::ok_type, T>
                  && library::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) &&
        {
            if (is_ok())
            {
                return U(result_ok, library::std::move(unwrap()));
            }

            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            );
        }

        template<
            typename Function,
            typename Ecvref = const E &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires is_result<U>
                  && library::std::is_same_v<typename U::ok_type, T>
                  && library::std::is_move_constructible_v<T>
        constexpr auto or_else(Function &&function) const &&
        {
            if (is_ok())
            {
                return U(result_ok, library::std::move(unwrap()));
            }
            return library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
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
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
            requires library::std::is_copy_constructible_v<E>
        constexpr auto transform(Function &&function) &
        {
            if (is_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            if constexpr (library::std::is_void_v<U>)
            {
                library::std::invoke(library::std::forward<Function>(function));
                return result<U, E>();
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(
                    library::std::forward<Function>(function),
                    library::std::move(unwrap())
                )
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
            requires library::std::is_copy_constructible_v<E>
        constexpr auto transform(Function &&function) const &
        {
            if (is_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            if constexpr (library::std::is_void_v<U>)
            {
                library::std::invoke(library::std::forward<Function>(function));
                return result<U, E>();
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(
                    library::std::forward<Function>(function),
                    library::std::move(unwrap())
                )
            );
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
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

            if constexpr (library::std::is_void_v<U>)
            {
                library::std::invoke(library::std::forward<Function>(function));
                return result<U, E>();
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(
                    library::std::forward<Function>(function),
                    library::std::move(unwrap())
                )
            );
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
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

            if constexpr (library::std::is_void_v<U>)
            {
                library::std::invoke(library::std::forward<Function>(function));
                return result<U, E>();
            }

            return result<U, E>(
                result_in_place,
                result_ok,
                library::std::invoke(
                    library::std::forward<Function>(function),
                    library::std::move(unwrap())
                )
            );
        }

        template<
            typename Function,
            typename Ecvref = E &,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) &
        {
            if (is_ok())
            {
                return result<T, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        unwrap()
                    )
                );
            }

            return result<T, F>(library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            ));
        }

        template<
            typename Function,
            typename Ecvref = const E &,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) const &
        {
            if (is_ok())
            {
                return result<T, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        unwrap()
                    )
                );
            }

            return result<T, F>(library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            ));
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) &&
        {
            if (is_ok())
            {
                return result<T, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        library::std::move(unwrap())
                    )
                );
            }

            return result<T, F>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            ));
        }

        template<
            typename Function,
            typename Ecvref = E const &&,
            typename F = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) const &&
        {
            if (is_ok())
            {
                return result<T, F>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        library::std::move(unwrap())
                    )
                );
            }

            return result<T, F>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            ));
        }

        // equality operators
        template<typename U, typename F>
            requires(!library::std::is_same_v<U, F>)
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
            requires library::std::
                is_convertible_v<library::std::remove_cvref_t<U>, T>
            friend constexpr bool operator==(const result &lhs, const U &rhs)
        {
            return lhs.is_ok() && static_cast<bool>(lhs.unwrap() == rhs);
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
