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

    enum class rights : a9n::word
    {
        READ    = 1 << 0,
        WRITE   = 1 << 1,
        EXECUTE = 1 << 2,
        ALL     = READ | WRITE | EXECUTE,
    };

    // Kernel mapping attributes; architecture-specific cache bits belong to HAL.
    enum class mapping_flag : a9n::word
    {
        NORMAL   = 0,
        BUFFERED = 1,
        UNCACHED = 2,
    };

    inline constexpr a9n::word MAPPING_FLAG_SHIFT = 4;
    inline constexpr a9n::word MAPPING_FLAG_MASK  = 0xf << MAPPING_FLAG_SHIFT;
    inline constexpr a9n::word MAPPING_ATTRIBUTE_MASK
        = static_cast<a9n::word>(rights::ALL) | MAPPING_FLAG_MASK;

    inline constexpr bool is_valid_mapping_attribute(a9n::word attribute)
    {
        return !(attribute & ~MAPPING_ATTRIBUTE_MASK)
            && ((attribute & MAPPING_FLAG_MASK) >> MAPPING_FLAG_SHIFT)
                   <= static_cast<a9n::word>(mapping_flag::UNCACHED);
    }

    inline constexpr mapping_flag get_mapping_flag(a9n::word attribute)
    {
        return static_cast<mapping_flag>((attribute & MAPPING_FLAG_MASK) >> MAPPING_FLAG_SHIFT);
    }

    inline constexpr a9n::word with_mapping_flag(a9n::word attribute, mapping_flag flag)
    {
        return (attribute & ~MAPPING_FLAG_MASK) | (static_cast<a9n::word>(flag) << MAPPING_FLAG_SHIFT);
    }

    struct page_table
    {
        a9n::physical_address address;
        a9n::word             flags;

        constexpr page_table(a9n::physical_address initial_address, a9n::word initial_depth)
            : address { initial_address }
            , flags { 0 }
        {
            configure_depth(initial_depth);
        }

        void configure_depth(uint8_t depth)
        {
            flags &= ~static_cast<a9n::word>(0xFF);
            flags |= depth;
        }

        uint8_t get_depth(void) const
        {
            return flags & 0xFF;
        }
    };

    struct frame
    {
        a9n::physical_address address;

        // 2^(12 + size) = page size
        // 2^12 = 4096 (most commonly used)
        // 2^21 = 2097152 (2MiB)
        // 2^30 = 1,073,741,824 (1GiB)
        a9n::word size_bits;

        // [0:7] depth, [8:15] rights
        a9n::word flags;
    };

    inline constexpr a9n::word FRAME_FLAG_DEVICE = 1ULL << 16;

    inline constexpr bool frame_is_device(const frame &target)
    {
        return (target.flags & FRAME_FLAG_DEVICE) != 0;
    }

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
