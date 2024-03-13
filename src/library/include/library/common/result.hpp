#ifndef LIBRARY_RESULT_HPP
#define LIBRARY_RESULT_HPP

#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

namespace library::common
{
    template<typename, typename>
    class result;

    template<typename T, int x>
    class GenericWrapper;

    template<typename T>
    using error_type = GenericWrapper<T, 0>;

    template<typename T>
    using value_type = GenericWrapper<T, 1>;

    template<typename T, int x>
    class GenericWrapper
    {
        T v;
        GenericWrapper(T &&a) : v(library::std::forward<T>(a)) {};
        GenericWrapper(const T &a) : v(a) {};
        GenericWrapper(const GenericWrapper &) = default;
        GenericWrapper(GenericWrapper &&) = default;

      public:
        ~GenericWrapper() = default;

        template<typename, typename>
        friend class result;

        template<typename U>
        friend auto error(U &&error)
            -> error_type<library::std::remove_cvref_t<U>>;

        template<typename U>
        friend auto value(U &&value)
            -> GenericWrapper<library::std::remove_cvref_t<U>, 1>;
    };

    template<typename T>
    auto error(T &&error) -> error_type<library::std::remove_cvref_t<T>>
    {
        return { library::std::forward<T>(error) };
    }

    template<typename T>
    auto value(T &&value) -> value_type<library::std::remove_cvref_t<T>>
    {
        return { library::std::forward<T>(value) };
    }

    template<typename T, typename ErrorT = int>
    class result
    {
        using Type = library::std::remove_cvref_t<T>;
        using Error = library::std::remove_cvref_t<ErrorT>;

        union
        {
            Type value;
            Error error;
        } m_stored;
        bool m_is_error = false;

      public:
        template<typename U>
        requires(library::std::is_same_v<
                 library::std::remove_cvref_t<
                     library::std::remove_pointer_t<U>>,
                 library::std::remove_cvref_t<
                     library::std::remove_pointer_t<T>>>)
            result(value_type<U> &&value)
        {
            new (&m_stored.value) T(library::std::move(value.v));
        }

        template<typename U>
        requires(library::std::is_same_v<
                 library::std::remove_cvref_t<
                     library::std::remove_pointer_t<U>>,
                 library::std::remove_cvref_t<
                     library::std::remove_pointer_t<ErrorT>>>)
            result(error_type<U> &&error)
        {
            m_is_error = true;
            new (&m_stored.error) ErrorT(library::std::move(error.v));
        }

        ~result()
        {
            if (is_error())
                m_stored.error.~ErrorT();
            else
                m_stored.value.~T();
        }

        result(const result &other)
        {
            if (other.is_error())
            {
                m_is_error = true;
                m_stored.error = other.get_error();
            }
            else
            {
                m_stored.value = other.get_value();
            }
        }

        result(result &&other)
        {
            if (other.is_error())
            {
                m_is_error = true;
                m_stored.error = library::std::move(other.get_error());
            }
            else
            {
                m_stored.value = library::std::move(other.get_value());
            }
        }

        bool is_error() const
        {
            return m_is_error;
        }

        bool is_value() const
        {
            return !is_error();
        }

        T &get_value()
        {
            const auto &as_const = *this;
            return const_cast<T &>(as_const.get_value());
        }

        const T &get_value() const
        {
            return m_stored.value;
        }

        ErrorT &get_error()
        {
            const auto &as_const = *this;
            return const_cast<ErrorT &>(as_const.get_error());
        }

        const ErrorT &get_error() const
        {
            return m_stored.error;
        }
    };
}

#endif
