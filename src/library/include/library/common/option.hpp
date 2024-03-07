#ifndef LIBH5N_OPTION_HPP
#define LIBH5N_OPTION_HPP

#include <library/libcxx/utility>

namespace library::common
{
    template<typename T>
    class option
    {
      public:
        option() noexcept
        {
        }

        explicit option(T value, bool has_value = false) noexcept
            : value_(std::move(value))
            , has_value_(has_value)
        {
        }

        option(const option &other) noexcept
            : value_(other.value_)
            , has_value_(other.has_value_)
        {
        }

        option &operator=(const option &other) noexcept
        {
            value_ = other.value_;
            has_value_ = other.has_value_;
            return *this;
        }

        option(option &&other) noexcept
            : value_(std::move(other.value_))
            , has_value_(other.has_value_)
        {
        }

        option &operator=(option &&other) noexcept
        {
            value_ = std::move(other.value_);
            has_value_ = other.has_value_;
            return *this;
        }

        bool is_none() const noexcept
        {
            return !has_value_;
        }

        bool has_value() const noexcept
        {
            return has_value_;
        }

        T &unwrap() &
        {
            if (!has_value_)
            {
            }
            return value_;
        }

        const T &unwrap() const &
        {
            if (!has_value_)
            {
            }
            return value_;
        }

        T unwrap_or(T default_value) const &
        {
            return has_value_ ? value_ : default_value;
        }

      private:
        bool has_value_;
        T value_;
    };

    template<typename T>
    option<T> some(T value)
    {
        return option<T>(std::move(value), true);
    }

    template<typename T>
    option<T> none()
    {
        return option<T>();
    }
}

#endif
