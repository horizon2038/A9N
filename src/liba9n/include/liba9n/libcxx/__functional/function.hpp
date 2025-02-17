#ifndef LIBCXX_FUNCTION_HPP
#define LIBCXX_FUNCTION_HPP

#include <liba9n/libcxx/__functional/invoke.hpp>
#include <liba9n/libcxx/__utility/move.hpp>
#include <stdint.h>

namespace liba9n::std
{
    namespace detail
    {
        inline constexpr uintmax_t FUNCTION_BUFFER_SIZE = 16;
    }

    template<typename T>
    class function;

    // this implementation limits the function object size to 16 bytes
    template<typename R, typename... Args>
    class function<R(Args...)>
    {
      public:
        template<typename F>
        function(F func) : callable { new(&buffer) callable_impl<F>(liba9n::std::move(func)) }
        {
        }

        R operator()(Args... args)
        {
            return callable->call(args...);
        }

      private:
        // type erasure base
        struct callable_base
        {
            virtual R call(Args...)  = 0;
            virtual ~callable_base() = default;
        };

        template<typename T>
        struct callable_impl : callable_base
        {
            callable_impl(T callable) : callable { liba9n::std::move(callable) }
            {
            }

            R call(Args... args)
            {
                return liba9n::std::invoke(callable, args...);
            }

            T callable;
        };

        uint8_t        buffer[detail::FUNCTION_BUFFER_SIZE];
        callable_base *callable;
    };
}

#endif
