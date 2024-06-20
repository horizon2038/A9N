#ifndef LIBRARY_RESULT_HPP
#define LIBRARY_RESULT_HPP

#include "library/libcxx/__type_traits/is_constructible.hpp"
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
    //
    // similar to std::expected<T, E>
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
            char dummy;
            T ok_value;
            E error_value;
        };
        bool has_value_flag;

      public:
        // conditionally trivial special member functions
        constexpr result(const result &other)
            requires(library::std::is_trivially_copy_constructible_v<T>
                     && library::std::is_trivially_copy_constructible_v<E>)
        = default;

        constexpr result(result &&other)
            requires(library::std::is_trivially_move_constructible_v<T>
                     && library::std::is_trivially_move_constructible_v<E>)
        = default;

        constexpr ~result()
            requires(library::std::is_trivially_destructible_v<T>
                     && library::std::is_trivially_destructible_v<E>)
        = default;

        constexpr result &operator=(result const &other)
            requires(library::std::is_trivially_copy_assignable_v<T>
                     && library::std::is_trivially_copy_assignable_v<E>)
        = default;

        constexpr result &operator=(result &&other)
            requires(library::std::is_trivially_move_assignable_v<T>
                     && library::std::is_trivially_move_assignable_v<E>)
        = default;

        // default constructor
        constexpr result() noexcept : dummy {}, has_value_flag { true }
        {
        }

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
            requires(!library::std::is_same_v<library::std::remove_cvref_t<U>, result<T, E>> && library::std::is_convertible_v<library::std::remove_cvref_t<U>, T>)
        constexpr result(U &&other) noexcept : has_value_flag(true)
        {
            new (&ok_value) T(library::std::forward<T>(other));
        }

        template<typename F>
            requires(!library::std::is_same_v<library::std::remove_cvref_t<F>, result<T, E>> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
        constexpr result(F &&other) noexcept : has_value_flag(false)
        {
            new (&error_value) E(library::std::forward<E>(other));
        }

        // obvious constructors
        //  - for future extensions
        //  (changes to allow the same type for T and E).
        template<typename U>
            requires(!library::std::is_same_v<library::std::remove_cvref_t<U>, result<T, E>> && library::std::is_convertible_v<library::std::remove_cvref_t<U>, T>)
        constexpr result(result_ok_tag, U &&other) noexcept
            : has_value_flag(true)
        {
            new (&ok_value) T(library::std::forward<T>(other));
        }

        template<typename F>
            requires(!library::std::is_same_v<library::std::remove_cvref_t<F>, result<T, E>> && library::std::is_convertible_v<library::std::remove_cvref_t<F>, E>)
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
                new (&error_value) E(other.unwrap_error);
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

        constexpr ~result() noexcept
        {
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
        // requires(library::std::is_convertible_v<T, U> &&
        // !library::std::is_same_v<result<T, E>,
        // library::std::remove_cvref_t<U>>)
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
            requires(library::std::is_convertible_v<U, T> && library::std::is_convertible_v<F, E>)
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
            requires(library::std::is_convertible_v<U, T> && library::std::is_convertible_v<F, E>)
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

        constexpr bool has_value() const noexcept
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
        constexpr auto &&unwrap_or(U &&value) &
        {
            if (!has_value())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return unwrap();
        }

        template<typename U>
            requires library::std::is_copy_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto &&unwrap_or(U &&value) const &
        {
            if (!has_value())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return unwrap();
        }

        template<typename U>
            requires library::std::is_move_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto &&unwrap_or(U &&value) &&
        {
            if (!has_value())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return library::std::move(unwrap());
        }

        template<typename U>
            requires library::std::is_move_constructible_v<U>
                  && library::std::is_convertible_v<U, T>
        constexpr auto &&unwrap_or(U &&value) const &&
        {
            if (!has_value())
            {
                return static_cast<T>(library::std::forward<U>(value));
            }

            return library::std::move(unwrap());
        }

        template<typename G = E>
            requires library::std::is_copy_constructible_v<E>
                  && library::std::is_convertible_v<G, E>
        constexpr auto &&unwrap_error_or(G &&value) &
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
        constexpr auto &&unwrap_error_or(G &&value) const &
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
        constexpr auto &&unwrap_error_or(G &&value) &&
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
        constexpr auto &&unwrap_error_or(G &&value) const &&
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
            if (!has_value())
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
            if (!has_value())
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
            if (!has_value())
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
            if (!has_value())
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
            if (!has_value())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            return result<U, E>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            ));
        }

        template<
            typename Function,
            typename Tcvref = const T &,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
        constexpr auto transform(Function &&function) const &
        {
            if (!has_value())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            return result<U, E>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            ));
        }

        template<
            typename Function,
            typename Tcvref = T &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
        constexpr auto transform(Function &&function) &&
        {
            if (!has_value())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            return result<U, E>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            ));
        }

        template<
            typename Function,
            typename Tcvref = const T &&,
            typename U = library::std::remove_cvref_t<
                library::std::invoke_result_t<Function, Tcvref>>>
        constexpr auto transform(Function &&function) const &&
        {
            if (!has_value())
            {
                return result<U, E>(
                    result_error,
                    library::std::move(unwrap_error())
                );
            }

            return result<U, E>(library::std::invoke(
                library::std::forward<Function>(function),
                library::std::move(unwrap())
            ));
        }

        // TODO: add monadic operations:
        // - transform_error
        //
        // TODO: add result<void, E> specialization
    };

    // deduction guide
    template<typename T, typename E>
        requires(library::std::is_same_v<T, E>)
    result(T, E) -> result<T, E>;

    template<typename T>
    result(T) -> result<T, void>;

    template<typename E>
    result(E) -> result<void, E>;

    template<typename T, typename E, typename... Args>
    constexpr result<T, E> make_result_ok(Args... args) noexcept
    {
        return result<T, E>(
            result_in_place,
            result_ok,
            static_cast<Args &&>(args)...
        );
    }

    template<typename T, typename E, typename... Args>
    constexpr result<T, E> make_result_error(Args... args) noexcept
    {
        return result<T, E>(
            result_in_place,
            result_error,
            static_cast<Args &&>(args)...
        );
    }
}

#endif
