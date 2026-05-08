#ifndef LIBA9N_ALLOCATOR_HPP
#define LIBA9N_ALLOCATOR_HPP

#include <liba9n/common/not_null.hpp>
#include <liba9n/libcxx/utility>
#include <liba9n/result/result.hpp>

namespace liba9n
{
    inline constexpr uintmax_t align_to(uintmax_t value, uintmax_t base)
    {
        return (value + base - 1) / base * base;
    }

    enum class allocator_error
    {
        OUT_OF_MEMORY,
        UNEXPECTED,
    };

    // simple linear allocator
    template<uintmax_t PoolSize>
    class linear_allocator
    {
      private:
        // fixed size buffer
        alignas(4096) uint8_t pool[PoolSize];
        // "heap" pointer
        uintmax_t current { 0 };

      public:
        template<typename T, typename... Args>
            requires(sizeof(T) < PoolSize)
        constexpr liba9n::result<liba9n::not_null<T>, allocator_error>
            allocate(uintmax_t count, Args &&...args)
        {
            auto      aligned_current = align_to(current, sizeof(T));
            uintmax_t total_size      = sizeof(T) * count;
            if (PoolSize < aligned_current || (PoolSize - aligned_current) < total_size)
            {
                return allocator_error::OUT_OF_MEMORY;
            }

            auto start = aligned_current;

            for (uintmax_t i = 0; i < count; i++)
            {
                new (reinterpret_cast<void *>(pool + aligned_current))
                    T(liba9n::std::forward<Args>(args)...);

                aligned_current += sizeof(T);
            }

            current              = aligned_current;

            T *allocated_pointer = reinterpret_cast<T *>(pool + start);
            if (!allocated_pointer)
            {
                return allocator_error::UNEXPECTED;
            }

            return not_null<T>(*allocated_pointer);
        }
    };
}

#endif
