#ifndef LIBH5N_RESULT_HPP
#define LIBH5N_RESULT_HPP

#include <library/libcxx/utility>

namespace library::common
{
    enum class result_state
    {
        VALUE_TYPE,
        ERROR_TYPE
    };

    template<typename T, typename E>
    class result
    {
      public:
        result() noexcept : state_(result_state::VALUE_TYPE), value_()
        {
        }
        explicit result(T value) noexcept
            : state_(result_state::VALUE_TYPE)
            , value_(std::move(value))
        {
        }
        explicit result(const E &error) noexcept
            : state_(result_state::ERROR_TYPE)
            , error_(error)
        {
        }

        bool has_value() const noexcept
        {
            return state_ == result_state::VALUE_TYPE;
        }
        bool is_error() const noexcept
        {
            return state_ == result_state::ERROR_TYPE;
        }

        const T &value() const &
        {
            if (state_ != result_state::VALUE_TYPE)
            {
            }
            return value_;
        }

        T &value() &
        {
            if (state_ != result_state::VALUE_TYPE)
            {
            }
            return value_;
        }

        const E &error_value() const &
        {
            if (state_ != result_state::ERROR_TYPE)
            {
            }
            return error_;
        }

        E &error_value() &
        {
            if (state_ != result_state::ERROR_TYPE)
            {
            }
            return error_;
        }

        static result<T, E> ok(T value) noexcept
        {
            return result<T, E>(std::move(value));
        }
        static result<T, E> error(const E &error) noexcept
        {
            return result<T, E>(error);
        }

      private:
        result_state state_;
        union
        {
            T value_;
            E error_;
        };
    };
}

#endif
