#ifndef LIBRARY_RESULT_HPP
#define LIBRARY_RESULT_HPP

#include <library/libcxx/functional>
#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

namespace library::common
{
    // tag
    struct result_in_place_tag
    {
    };

    inline constexpr result_in_place_tag result_in_place;

    struct result_ok_tag
    {
    };

    inline constexpr result_ok_tag result_ok;

    struct result_error_tag
    {
    };

    inline constexpr result_error_tag result_error;

    // result<T, E> :
    // similar to std::expected<T, E>.
    // however, to simplify type inference,
    // we require that T and E be completely different types.
    template<typename T, typename E>
        requires(!library::std::is_same_v<T, E>)
    class result;

    template<typename T>
    concept is_result = library::std::is_same_v<
        library::std::remove_cvref_t<T>,
        result<typename T::ok_type, typename T::error_type>>;

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
        bool has_value_flag;

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
            , has_value_flag { true }
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
            , has_value_flag { false }
        {
            new (&error_value) E(static_cast<Args &&>(args)...);
        }

        // deduction constructor
        template<typename U>
            requires(!is_result<U> && library::std::is_convertible_v<library::std::remove_cvref_t<U>, T>)
        constexpr result(U &&other) noexcept : has_value_flag(true)
        {
            new (&ok_value) T(library::std::forward<T>(other));
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(F &&other) noexcept : has_value_flag(false)
        {
            new (&error_value) E(library::std::forward<E>(other));
        }

        // obvious constructors
        //  - for future extensions
        //  (changes to allow the same type for T and E).
        template<typename U>
            requires(!is_result<U> && library::std::is_convertible_v<library::std::remove_cvref_t<U>, T>)
        constexpr result(result_ok_tag, U &&other) noexcept
            : has_value_flag(true)
        {
            new (&ok_value) T(library::std::forward<T>(other));
        }

        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(result_error_tag, F &&other) noexcept
            : has_value_flag(false)
        {
            new (&error_value) E(library::std::forward<E>(other));
        }

        constexpr result(const result &other)
            : has_value_flag { other.has_value() }
        {
            if (other.has_value())
            {
                new (&ok_value) T(other.unwrap());
            }
            else
            {
                new (&error_value) E(other.unwrap_error());
            }
        }

        constexpr result(result &&other) : has_value_flag { other.has_value() }
        {
            if (other.has_value())
            {
                new (&ok_value) T(library::std::move(other.unwrap()));
            }
            else
            {
                new (&error_value) E(library::std::move(other.unwrap_error()));
            }

            other.has_value_flag = false;
        }

        // add result<U, F> constructor

        constexpr ~result() noexcept
        {
            // remove it
            if (!has_value_flag)
            {
                return;
            }

            if constexpr (!library::std::is_trivially_destructible_v<T>)
            {
                ok_value.~T();
            }

            if constexpr (!library::std::is_trivially_destructible_v<E>)
            {
                error_value.~E();
            }
        }

        template<typename U>
            requires(!is_result<U> && library::std::is_convertible_v<U, T>)
        constexpr result &operator=(U &&u) noexcept
        {
            new (&ok_value) T(static_cast<T>(library::std::forward<U>(u)));
            has_value_flag = true;

            return *this;
        }

        constexpr result &operator=(const result &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value();
            if (other.has_value())
            {
                new (&ok_value) T(other.unwrap());
            }
            else
            {
                new (&error_value) T(other.unwrap_error());
            }

            return *this;
        }

        constexpr result &operator=(result &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value();
            if (other.has_value())
            {
                new (&ok_value) T(library::std::move(other.unwrap()));
            }
            else
            {
                new (&error_value) E(library::std::move(other.unwrap_error()));
            }
            other.has_value_flag = false;

            return *this;
        }

        template<typename U, typename F>
            requires library::std::is_convertible_v<U, T>
                  && library::std::is_convertible_v<F, E>
        constexpr result &operator=(const result<U, F> &other)
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value();
            if (other.has_value())
            {
                new (&ok_value) T(static_cast<T>(other.unwrap()));
            }
            else
            {
                new (&error_value) E(static_cast<E>(other.unwrap_error()));
            }

            return *this;
        }

        template<typename U, typename F>
            requires library::std::is_convertible_v<U, T>
                  && library::std::is_convertible_v<F, E>
        constexpr result &operator=(result<U, F> &&other)
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value();
            if (other.has_value())
            {
                new (&ok_value)
                    T(static_cast<T>(library::std::move(other.unwrap())));
            }
            else
            {
                new (&error_value)
                    E(static_cast<E>(library::std::move(other.unwrap_error())));
            }
            other.has_value_flag = false;

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
            return has_value_flag;
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

        [[nodiscard("result::has_value() must be used.")]] constexpr bool
            has_value() const noexcept
        {
            return has_value_flag;
        }

        constexpr bool has_error() const noexcept
        {
            return !has_value_flag;
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&value) &
        {
            if (has_error())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return unwrap();
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&value) const &
        {
            if (has_error())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return unwrap();
        }

        template<typename U>
            requires library::std::is_move_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&value) &&
        {
            if (has_error())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return library::std::move(unwrap());
        }

        template<typename U>
            requires library::std::is_move_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto unwrap_or(U &&value) const &&
        {
            if (has_error())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return library::std::move(unwrap());
        }

        template<typename G = E>
            requires library::std::is_copy_constructible_v<E>
                  && library::std::is_convertible_v<G, E>
        constexpr auto unwrap_error_or(G &&value) &
        {
            if (has_value())
            {
                return static_cast<E>(library::std::forward<G>(value));
            }

            return unwrap_error();
        }

        template<typename G = E>
            requires library::std::is_move_constructible_v<E>
                  && library::std::is_convertible_v<G, E>
        constexpr auto unwrap_error_or(G &&value) const &
        {
            if (has_value())
            {
                return static_cast<E>(library::std::forward<G>(value));
            }

            return unwrap_error();
        }

        template<typename G = E>
            requires library::std::is_copy_constructible_v<E>
                  && library::std::is_convertible_v<G, E>
        constexpr auto unwrap_error_or(G &&value) &&
        {
            if (has_value())
            {
                return static_cast<E>(library::std::forward<G>(value));
            }

            return library::std::move(unwrap_error());
        }

        template<typename G = E>
            requires library::std::is_move_constructible_v<E>
                  && library::std::is_convertible_v<G, E>
        constexpr auto unwrap_error_or(G &&value) const &&
        {
            if (has_value())
            {
                return static_cast<E>(library::std::forward<G>(value));
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
            if (has_error())
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
            if (has_error())
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
            if (has_error())
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
            if (has_error())
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
            static_assert(
                library::std::is_same_v<typename U::ok_type, T>,
                "Function must return result<T, E>"
            );
            if (has_value())
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
            static_assert(
                library::std::is_same_v<typename U::ok_type, T>,
                "Function must return result<T, E>"
            );
            if (has_value())
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
            static_assert(
                library::std::is_same_v<typename U::ok_type, T>,
                "Function must return result<T, E>"
            );
            if (has_value())
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
            static_assert(
                library::std::is_same_v<typename U::ok_type, T>,
                "Function must return result<T, E>"
            );

            if (has_value())
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
        constexpr auto transform(Function &&function) &
        {
            if (has_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
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
        constexpr auto transform(Function &&function) const &
        {
            if (has_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
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
        constexpr auto transform(Function &&function) &&
        {
            if (has_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
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
        constexpr auto transform(Function &&function) const &&
        {
            if (has_error())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
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
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_same_v<E, U>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) &
        {
            if (has_value())
            {
                return result<T, U>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        unwrap()
                    )
                );
            }

            return result<T, U>(library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            ));
        }

        template<
            typename Function,
            typename Ecvref = const E &,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_same_v<E, U>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) const &
        {
            if (has_value())
            {
                return result<T, U>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        unwrap()
                    )
                );
            }

            return result<T, U>(library::std::invoke(
                library::std::forward<Function>(function),
                unwrap_error()
            ));
        }

        template<
            typename Function,
            typename Ecvref = E &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_same_v<E, U>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) &&
        {
            if (has_value())
            {
                return result<T, U>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        library::std::move(unwrap())
                    )
                );
            }

            return result<T, U>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            ));
        }

        template<
            typename Function,
            typename Ecvref = E const &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Ecvref>>>
            requires library::std::is_same_v<E, U>
                  && library::std::is_copy_constructible_v<T>
        constexpr auto transform_error(Function &&function) const &&
        {
            if (has_value())
            {
                return result<T, U>(
                    result_in_place,
                    result_ok,
                    library::std::invoke(
                        library::std::forward<Function>(function),
                        library::std::move(unwrap())
                    )
                );
            }

            return result<T, U>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap_error())
            ));
        }
    };

    // TODO:
    // - add or_else overload for T is *void* to result<T, E>
    // - add template specialization of result<void, E>

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
        bool has_value_flag;

        // helper methods

        /*
        template<typename U>
            requires library::std::is_convertible_v<U, T>
        constexpr void update_ok_value(U &&new_value)
        {
            init_ok_value();
            new (&error_value)
                T(static_cast<T>(library::std::forward<U>(new_ok_value)));
        }

        constexpr void init_ok_value()
        {
            if (has_error())
            {
                return;
            }

            if constexpr (library::std::is_trivially_destructible_v<T>)
            {
                return;
            }

            ok_value.~T();
        }
        */

        template<typename F>
            requires library::std::is_convertible_v<F, E>
        constexpr void update_error_value(F &&new_error)
        {
            init_error_value();
            new (&error_value)
                E(static_cast<E>(library::std::forward<F>(new_error)));
        }

        constexpr void init_error_value()
        {
            // has_value()
            if (has_value_flag)
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
        constexpr result() noexcept : dummy {}, has_value_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename... Args>
        constexpr result(
            [[maybe_unused]] result_in_place_tag,
            [[maybe_unused]] result_ok_tag,
            Args... args
        ) noexcept
            : dummy {}
            , has_value_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename... Args>
        constexpr result(
            [[maybe_unused]] result_in_place_tag,
            [[maybe_unused]] result_error_tag,
            Args... args
        ) noexcept
            : has_value_flag { false }
        {
            new (&error_value) E(static_cast<Args &&>(args)...);
        }

        template<typename U = void>
            requires(!is_result<U> && library::std::is_convertible_v<U, void>)
        constexpr result(U &&value) noexcept : dummy {}
                                             , has_value_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename F = E>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(F &&error) noexcept : has_value_flag { false }
        {
            new (error_value) E(library::std::forward<F>(error));
        }

        // obvious constructors
        // - for future extensions
        template<typename U>
            requires(!is_result<U>
                     && library::std::is_convertible_v<
                         library::std::remove_cvref_t<U>,
                         void>)
        constexpr result(result_ok_tag, U &&value) noexcept
            : dummy {}
            , has_value_flag { true }
        {
            // holds the valid value : `void`
        }

        template<typename F = E>
            requires(!is_result<F> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(result_error_tag, F &&error) noexcept
            : has_value_flag { false }
        {
            new (error_value) E(library::std::forward<F>(error));
        }

        // result<T, E> -> result<T, E>
        constexpr result(const result &other)
            : dummy {}
            , has_value_flag { other.has_value() }
        {
            if (other.has_value())
            {
                return;
            }

            new (&error_value) E(other.unwrap_error());
        }

        constexpr result(result &&other)
            : dummy {}
            , has_value_flag { other.has_value() }
        {
            if (other.has_error())
            {
                new (&error_value) E(library::std::move(other.unwrap_error()));
            }

            other.has_value_flag = false;
        }

        // result<U, F> -> result<T, E>
        template<typename U>
            requires is_result<U>
                      && library::std::
                             is_convertible_v<typename U::ok_type, void>
                      && library::std::
                             is_convertible_v<typename U::error_type, E>
        constexpr result(const U &other)
            : dummy {}
            , has_value_flag { other.has_value() }
        {
            if (other.has_value())
            {
                return;
            }

            new (&error_value) E(static_cast<E>(other.unwrap_error()));
        }

        template<typename U>
            requires is_result<U>
                      && library::std::
                             is_convertible_v<typename U::ok_type, void>
                      && library::std::
                             is_convertible_v<typename U::error_type, E>
        constexpr result(U &&other)
            : dummy {}
            , has_value_flag { other.has_value() }
        {
            if (other.has_error())
            {
                new (&error_value)
                    E(static_cast<E>(library::std::move(other.unwrap_error())));
            }

            other.has_value_flag = false;
        }

        constexpr ~result() noexcept
        {
            // `E` is not trivial
            error_value.~E();
        }

        // operators
        template<typename F>
            requires(!is_result<F> && library::std::is_convertible_v<F, E>)
        constexpr result &operator=(F &&new_error) noexcept
        {
            update_error_value(library::std::forward<F>(new_error));
            has_value_flag = false;

            return *this;
        }

        constexpr result &operator=(const result &other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            has_value_flag = other.has_value();
            if (other.has_error())
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

            has_value_flag = other.has_value();
            if (other.has_error())
            {
                update_error_value(library::std::move<E>(other.unwrap_error()));
                other.has_value_flag = false;
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

            has_value_flag = other.has_value();
            if (other.has_error())
            {
                update_error_value(static_cast<E>(other.unwrap_error()));
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

            has_value_flag = other.has_value();
            if (other.has_error())
            {
                update_error_value(
                    static_cast<E>(library::std::move<E>(other.unwrap_error()))
                );
                other.has_value_flag = false;
            }

            return *this;
        }
    };

    // TODO:
    // - implementing result<void, E> specialization
    // - exact destruct within operator=

    // deduction guide
    template<typename T, typename E>
        requires(!library::std::is_same_v<T, E>)
    result(T, E) -> result<T, E>;

    template<typename T>
    result(T) -> result<T, void>;

    template<typename E>
    result(E) -> result<void, E>;

    template<typename T, typename E, typename... Args>
        requires(!library::std::is_same_v<T, E>)
    constexpr result<T, E> make_result_ok(Args... args) noexcept
    {
        return result<T, E>(
            result_in_place,
            result_ok,
            static_cast<Args &&>(args)...
        );
    }

    template<typename T, typename E, typename... Args>
        requires(!library::std::is_same_v<T, E>)
    constexpr result<T, E> make_result_error(Args... args) noexcept
    {
        return result<T, E>(
            result_in_place,
            result_error,
            static_cast<Args &&>(args)...
        );
    }
}

/*
 _                _                ____  _    _____  ___
| |__   ___  _ __(_)_______  _ __ |___ \| | _|___ / ( _ )
| '_ \ / _ \| '__| |_  / _ \| '_ \  __) | |/ / |_ \ / _ \
| | | | (_) | |  | |/ / (_) | | | |/ __/|   < ___) | (_) |
|_| |_|\___/|_|  |_/___\___/|_| |_|_____|_|\_\____/ \___/

*/

#endif
