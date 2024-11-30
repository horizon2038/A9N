#ifndef LIBA9N_ALLOCATOR_HPP
#define LIBA9N_ALLOCATOR_HPP

#include <kernel/types.hpp>
#include <liba9n/common/not_null.hpp>
#include <liba9n/libcxx/utility>
#include <liba9n/result/result.hpp>

#include <kernel/utility/logger.hpp>

namespace liba9n
{
    inline constexpr a9n::word align_to(a9n::word value, a9n::word base)
    {
        return (value + base - 1) / base * base;
    }

    enum class allocator_error
    {
        OUT_OF_MEMORY,
        UNEXPECTED,
    };

    // simple linear allocator
    template<a9n::word PoolSize>
    class linear_allocator
    {
      private:
        // fixed size buffer
        alignas(a9n::PAGE_SIZE) uint8_t pool[PoolSize];
        // "heap" pointer
        a9n::word current { 0 };

      public:
        template<typename T, typename... Args>
            requires(sizeof(T) < PoolSize)
        constexpr liba9n::result<liba9n::not_null<T>, allocator_error>
            allocate(a9n::word count, Args &&...args)
        {
            using a9n::kernel::utility::logger;

            logger::printk("linear_allocator::allocate\n");

            auto      aligned_current = align_to(current, sizeof(T));
            a9n::word total_size      = sizeof(T) * count;
            if (PoolSize < aligned_current || (PoolSize - aligned_current) < total_size)
            {
                return allocator_error::OUT_OF_MEMORY;
            }

            logger::printk("aligned_current : 0x%llx, total_size : 0x%llx\n", aligned_current, total_size);

            auto start = aligned_current;

            for (a9n::word i = 0; i < count; i++)
            {
                logger::printk(
                    "[%4llu] allocate to 0x%16llx\n",
                    i,
                    reinterpret_cast<void *>(pool + aligned_current)
                );

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
