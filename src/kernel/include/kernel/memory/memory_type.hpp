#ifndef MEMORY_TYPE_H
#define MEMORY_TYPE_H

#include <kernel/types.hpp>
#include <liba9n/result/result.hpp>

#include <stdbool.h>
#include <stdint.h>

namespace a9n::kernel
{
    enum class memory_map_type
    {
        FREE,
        DEVICE,
        RESERVED,
    };

    struct memory_map_entry
    {
        a9n::physical_address start_physical_address;
        a9n::word             page_count;
        memory_map_type       type;
    };

    struct memory_info
    {
        a9n::word         memory_size;
        uint16_t          memory_map_count;
        memory_map_entry *memory_map;
    };

    struct page_table
    {
        a9n::physical_address address;
        a9n::word             flags;

        constexpr page_table(
            a9n::physical_address initial_address,
            a9n::word             initial_depth,
            a9n::word             initial_rights = flag_type::ALL
        )
            : address { initial_address }
            , flags { 0 }
        {
            configure_depth(initial_depth);
            configure_rights(initial_rights);
        }

        // architecture-independent attribute
        // TODO: rename to "rights"
        enum flag_type : uint8_t
        {
            NONE    = 0,
            READ    = 1 << 1,
            WRITE   = 1 << 2,
            EXECUTE = 1 << 3,
            ALL     = READ | WRITE | EXECUTE,
        };

        void configure_depth(uint8_t depth)
        {
            flags &= ~static_cast<a9n::word>(0xFF);
            flags |= depth;
        }

        uint8_t get_depth(void) const
        {
            return flags & 0xFF;
        }

        void configure_rights(uint8_t rights)
        {
            flags &= ~static_cast<a9n::word>(0xFF00);
            flags |= (static_cast<a9n::word>(rights) << 8);
        }

        uint8_t get_rights(void) const
        {
            return (flags >> 8) & 0xFF;
        }
    };

    struct frame
    {
        a9n::physical_address address;

        // 2^(12 + size) = page size
        // 2^12 = 4096 (most commonly used)
        // 2^21 = 2097152 (2MiB)
        // 2^30 = 1,073,741,824 (1GiB)

        // [0:7] depth, [8:15] rights
        a9n::word flags;
    };

    enum class memory_map_error
    {
        ILLEGAL_DEPTH,
        ILLEGAL_AUTORITY,
        INVALID_PAGE_TABLE,
        INVALID_FRAME,
        ALREADY_MAPPED,
        NO_SUCH_PAGE_TABLE,
    };

    template<typename T = void>
    using memory_map_result = liba9n::result<T, memory_map_error>;

    static_assert(sizeof(page_table) <= (sizeof(a9n::word) * 3));
    static_assert(sizeof(frame) <= (sizeof(a9n::word) * 3));

    enum class memory_error
    {
        OUT_OF_MEMORY,
        INVALID_ADDRESS,
        INVALID_ALIGNMENT,
    };

    template<typename T = void>
    using memory_result = liba9n::result<T, memory_error>;
}

#endif
