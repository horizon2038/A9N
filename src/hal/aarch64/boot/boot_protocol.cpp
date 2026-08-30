#include <hal/arch/arch_types.hpp>

#include <kernel/boot/boot_info.hpp>
#include <kernel/memory/memory_type.hpp>
#include <kernel/types.hpp>

#include <stddef.h>
#include <stdint.h>

namespace
{
    using a9n::kernel::memory_map_entry;
    using a9n::kernel::memory_map_type;

    constexpr uint32_t  FDT_MAGIC           = 0xd00dfeed;
    constexpr uint32_t  FDT_BEGIN_NODE      = 1;
    constexpr uint32_t  FDT_END_NODE        = 2;
    constexpr uint32_t  FDT_PROPERTY        = 3;
    constexpr uint32_t  FDT_NOP             = 4;
    constexpr uint32_t  FDT_END             = 9;
    constexpr uint32_t  ELF_LOAD            = 1;
    constexpr uint32_t  ELF_SYMBOL_TABLE    = 2;
    constexpr uint16_t  ELF_MACHINE_AARCH64 = 183;
    constexpr a9n::word DIRECT_MAP_END      = 16ULL * 1024 * 1024 * 1024;
    constexpr size_t    INIT_IMAGE_CAPACITY = 64ULL * 1024 * 1024;
    constexpr size_t    RANGE_CAPACITY      = 32;
    constexpr size_t    MEMORY_MAP_CAPACITY = 64;

    struct physical_range
    {
        a9n::physical_address start;
        a9n::physical_address end;
    };

    alignas(a9n::PAGE_SIZE) uint8_t g_init_image[INIT_IMAGE_CAPACITY];
    a9n::kernel::boot_info g_boot_info;
    memory_map_entry       g_memory_map[MEMORY_MAP_CAPACITY];
    physical_range         g_ram_ranges[RANGE_CAPACITY];
    physical_range         g_reserved_ranges[RANGE_CAPACITY];
    size_t                 g_ram_range_count;
    size_t                 g_reserved_range_count;
    size_t                 g_memory_map_count;

    extern "C"
    {
        extern uint8_t __kernel_image_start_virtual[];
        extern uint8_t __kernel_end_virtual[];
    }

    constexpr a9n::word align_down(a9n::word value)
    {
        return value & ~(a9n::PAGE_SIZE - 1);
    }

    constexpr a9n::word align_up(a9n::word value)
    {
        return (value + a9n::PAGE_SIZE - 1) & ~(a9n::PAGE_SIZE - 1);
    }

    template<typename T>
    T *physical_pointer(a9n::physical_address address)
    {
        return reinterpret_cast<T *>(address | a9n::hal::KERNEL_VIRTUAL_BASE);
    }

    template<typename T>
    a9n::physical_address physical_address_of(T *pointer)
    {
        return reinterpret_cast<a9n::word>(pointer) & ~a9n::hal::KERNEL_VIRTUAL_BASE;
    }

    bool valid_span(a9n::word offset, a9n::word size, a9n::word total)
    {
        return offset <= total && size <= total - offset;
    }

    uint16_t read_little_u16(const uint8_t *source)
    {
        return static_cast<uint16_t>(source[0]) | static_cast<uint16_t>(source[1]) << 8;
    }

    uint32_t read_little_u32(const uint8_t *source)
    {
        return static_cast<uint32_t>(source[0]) | static_cast<uint32_t>(source[1]) << 8
             | static_cast<uint32_t>(source[2]) << 16 | static_cast<uint32_t>(source[3]) << 24;
    }

    uint64_t read_little_u64(const uint8_t *source)
    {
        return static_cast<uint64_t>(read_little_u32(source))
             | static_cast<uint64_t>(read_little_u32(source + 4)) << 32;
    }

    uint32_t read_big_u32(const uint8_t *source)
    {
        return static_cast<uint32_t>(source[0]) << 24 | static_cast<uint32_t>(source[1]) << 16
             | static_cast<uint32_t>(source[2]) << 8 | static_cast<uint32_t>(source[3]);
    }

    uint64_t read_big_u64(const uint8_t *source)
    {
        return static_cast<uint64_t>(read_big_u32(source)) << 32
             | static_cast<uint64_t>(read_big_u32(source + 4));
    }

    bool string_equals(const char *left, const char *right)
    {
        while (*left && *right)
        {
            if (*left++ != *right++)
            {
                return false;
            }
        }
        return *left == *right;
    }

    bool node_name_is(const char *node, const char *name)
    {
        while (*name)
        {
            if (*node++ != *name++)
            {
                return false;
            }
        }
        return *node == '\0' || *node == '@';
    }

    bool bounded_string_equals(const uint8_t *table, size_t table_size, uint32_t offset, const char *name)
    {
        if (offset >= table_size)
        {
            return false;
        }
        for (size_t index = offset; index < table_size; ++index, ++name)
        {
            if (table[index] != static_cast<uint8_t>(*name))
            {
                return false;
            }
            if (!*name)
            {
                return true;
            }
        }
        return false;
    }

    void copy_bytes(uint8_t *destination, const uint8_t *source, size_t size)
    {
        if (destination < source || destination >= source + size)
        {
            for (size_t index = 0; index < size; ++index)
            {
                destination[index] = source[index];
            }
            return;
        }

        for (size_t index = size; index != 0; --index)
        {
            destination[index - 1] = source[index - 1];
        }
    }

    void clear_bytes(uint8_t *destination, size_t size)
    {
        for (size_t index = 0; index < size; ++index)
        {
            destination[index] = 0;
        }
    }

    void add_range(physical_range ranges[], size_t &count, a9n::word start, a9n::word end)
    {
        start = align_down(start);
        end   = align_up(end);
        if (start >= end || count >= RANGE_CAPACITY)
        {
            return;
        }
        ranges[count++] = physical_range { start, end };
    }

    void sort_and_merge_ranges(physical_range ranges[], size_t &count)
    {
        for (size_t outer = 0; outer < count; ++outer)
        {
            for (size_t inner = outer + 1; inner < count; ++inner)
            {
                if (ranges[inner].start < ranges[outer].start)
                {
                    const auto temporary = ranges[outer];
                    ranges[outer]        = ranges[inner];
                    ranges[inner]        = temporary;
                }
            }
        }

        size_t merged = 0;
        for (size_t index = 0; index < count; ++index)
        {
            if (!merged || ranges[merged - 1].end < ranges[index].start)
            {
                ranges[merged++] = ranges[index];
                continue;
            }
            if (ranges[index].end > ranges[merged - 1].end)
            {
                ranges[merged - 1].end = ranges[index].end;
            }
        }
        count = merged;
    }

    uint64_t read_cells(const uint8_t *data, uint32_t cell_count)
    {
        if (!cell_count || cell_count > 2)
        {
            return 0;
        }
        uint64_t value = 0;
        for (uint32_t index = 0; index < cell_count; ++index)
        {
            value = value << 32 | read_big_u32(data + index * sizeof(uint32_t));
        }
        return value;
    }

    void parse_reg_property(
        const uint8_t *data,
        uint32_t       length,
        uint32_t       address_cells,
        uint32_t       size_cells,
        physical_range ranges[],
        size_t        &range_count
    )
    {
        const uint32_t tuple_cells = address_cells + size_cells;
        if (!tuple_cells || address_cells > 2 || size_cells > 2)
        {
            return;
        }
        const uint32_t tuple_size = tuple_cells * sizeof(uint32_t);
        for (uint32_t offset = 0; offset + tuple_size <= length; offset += tuple_size)
        {
            const auto start = read_cells(data + offset, address_cells);
            const auto size = read_cells(data + offset + address_cells * sizeof(uint32_t), size_cells);
            if (start < DIRECT_MAP_END && size && size <= DIRECT_MAP_END - start)
            {
                add_range(ranges, range_count, start, start + size);
            }
        }
    }

    enum class root_node_kind
    {
        NONE,
        MEMORY,
        CHOSEN,
        RESERVED_MEMORY,
    };

    bool parse_fdt(
        a9n::physical_address  dtb_address,
        a9n::physical_address &initrd_start,
        a9n::physical_address &initrd_end,
        a9n::word             &dtb_size
    )
    {
        if (!dtb_address || dtb_address >= DIRECT_MAP_END)
        {
            return false;
        }

        const auto *fdt = physical_pointer<const uint8_t>(dtb_address);
        if (read_big_u32(fdt) != FDT_MAGIC)
        {
            return false;
        }

        const uint32_t total_size         = read_big_u32(fdt + 4);
        const uint32_t structure_offset   = read_big_u32(fdt + 8);
        const uint32_t strings_offset     = read_big_u32(fdt + 12);
        const uint32_t reserve_map_offset = read_big_u32(fdt + 16);
        const uint32_t strings_size       = read_big_u32(fdt + 32);
        const uint32_t structure_size     = read_big_u32(fdt + 36);
        if (total_size < 40 || dtb_address > DIRECT_MAP_END - total_size
            || !valid_span(structure_offset, structure_size, total_size)
            || !valid_span(strings_offset, strings_size, total_size)
            || reserve_map_offset >= total_size)
        {
            return false;
        }
        dtb_size = total_size;

        for (uint32_t offset = reserve_map_offset; valid_span(offset, 16, total_size); offset += 16)
        {
            const auto start = read_big_u64(fdt + offset);
            const auto size  = read_big_u64(fdt + offset + 8);
            if (!start && !size)
            {
                break;
            }
            if (start < DIRECT_MAP_END && size && size <= DIRECT_MAP_END - start)
            {
                add_range(g_reserved_ranges, g_reserved_range_count, start, start + size);
            }
        }

        const auto    *structure     = fdt + structure_offset;
        const auto    *structure_end = structure + structure_size;
        const auto    *strings       = fdt + strings_offset;
        const uint8_t *cursor        = structure;
        int            depth         = -1;
        uint32_t       address_cells = 2;
        uint32_t       size_cells    = 1;
        root_node_kind root_kind     = root_node_kind::NONE;

        while (cursor + sizeof(uint32_t) <= structure_end)
        {
            const auto token  = read_big_u32(cursor);
            cursor           += sizeof(uint32_t);
            if (token == FDT_BEGIN_NODE)
            {
                const auto *name = reinterpret_cast<const char *>(cursor);
                while (cursor < structure_end && *cursor)
                {
                    ++cursor;
                }
                if (cursor >= structure_end)
                {
                    return false;
                }
                ++cursor;
                cursor = reinterpret_cast<const uint8_t *>(
                    (reinterpret_cast<uintptr_t>(cursor) + 3) & ~static_cast<uintptr_t>(3)
                );
                ++depth;
                if (depth == 1)
                {
                    if (node_name_is(name, "memory"))
                    {
                        root_kind = root_node_kind::MEMORY;
                    }
                    else if (node_name_is(name, "chosen"))
                    {
                        root_kind = root_node_kind::CHOSEN;
                    }
                    else if (node_name_is(name, "reserved-memory"))
                    {
                        root_kind = root_node_kind::RESERVED_MEMORY;
                    }
                    else
                    {
                        root_kind = root_node_kind::NONE;
                    }
                }
                continue;
            }
            if (token == FDT_END_NODE)
            {
                if (depth == 1)
                {
                    root_kind = root_node_kind::NONE;
                }
                --depth;
                continue;
            }
            if (token == FDT_NOP)
            {
                continue;
            }
            if (token == FDT_END)
            {
                break;
            }
            if (token != FDT_PROPERTY || cursor + 8 > structure_end)
            {
                return false;
            }

            const uint32_t length       = read_big_u32(cursor);
            const uint32_t name_offset  = read_big_u32(cursor + 4);
            cursor                     += 8;
            if (static_cast<size_t>(structure_end - cursor) < length || name_offset >= strings_size)
            {
                return false;
            }
            const auto *data = cursor;

            if (depth == 0 && length == 4
                && bounded_string_equals(strings, strings_size, name_offset, "#address-cells"))
            {
                address_cells = read_big_u32(data);
            }
            else if (
                depth == 0 && length == 4
                && bounded_string_equals(strings, strings_size, name_offset, "#size-cells")
            )
            {
                size_cells = read_big_u32(data);
            }
            else if (
                depth == 1 && root_kind == root_node_kind::MEMORY
                && bounded_string_equals(strings, strings_size, name_offset, "reg")
            )
            {
                parse_reg_property(data, length, address_cells, size_cells, g_ram_ranges, g_ram_range_count);
            }
            else if (
                depth == 2 && root_kind == root_node_kind::RESERVED_MEMORY
                && bounded_string_equals(strings, strings_size, name_offset, "reg")
            )
            {
                parse_reg_property(
                    data,
                    length,
                    address_cells,
                    size_cells,
                    g_reserved_ranges,
                    g_reserved_range_count
                );
            }
            else if (
                depth == 1 && root_kind == root_node_kind::CHOSEN
                && bounded_string_equals(strings, strings_size, name_offset, "linux,initrd-start")
            )
            {
                initrd_start = read_cells(data, length / sizeof(uint32_t));
            }
            else if (
                depth == 1 && root_kind == root_node_kind::CHOSEN
                && bounded_string_equals(strings, strings_size, name_offset, "linux,initrd-end")
            )
            {
                initrd_end = read_cells(data, length / sizeof(uint32_t));
            }

            cursor += length;
            cursor  = reinterpret_cast<const uint8_t *>(
                (reinterpret_cast<uintptr_t>(cursor) + 3) & ~static_cast<uintptr_t>(3)
            );
        }

        return true;
    }

    bool find_elf_symbol(const uint8_t *elf, size_t elf_size, const char *symbol_name, a9n::word &symbol_value)
    {
        const auto section_offset     = read_little_u64(elf + 40);
        const auto section_entry_size = read_little_u16(elf + 58);
        const auto section_count      = read_little_u16(elf + 60);
        if (section_entry_size < 64
            || !valid_span(section_offset, static_cast<a9n::word>(section_entry_size) * section_count, elf_size))
        {
            return false;
        }

        for (uint16_t index = 0; index < section_count; ++index)
        {
            const auto *section = elf + section_offset + static_cast<size_t>(index) * section_entry_size;
            if (read_little_u32(section + 4) != ELF_SYMBOL_TABLE)
            {
                continue;
            }
            const auto symbol_offset     = read_little_u64(section + 24);
            const auto symbol_size       = read_little_u64(section + 32);
            const auto string_index      = read_little_u32(section + 40);
            const auto symbol_entry_size = read_little_u64(section + 56);
            if (string_index >= section_count || symbol_entry_size < 24
                || !valid_span(symbol_offset, symbol_size, elf_size))
            {
                continue;
            }

            const auto *string_section
                = elf + section_offset + static_cast<size_t>(string_index) * section_entry_size;
            const auto string_offset = read_little_u64(string_section + 24);
            const auto string_size   = read_little_u64(string_section + 32);
            if (!valid_span(string_offset, string_size, elf_size))
            {
                continue;
            }
            const auto *strings = elf + string_offset;

            for (a9n::word offset  = 0; offset + symbol_entry_size <= symbol_size;
                 offset           += symbol_entry_size)
            {
                const auto *symbol      = elf + symbol_offset + offset;
                const auto  name_offset = read_little_u32(symbol);
                if (bounded_string_equals(strings, string_size, name_offset, symbol_name))
                {
                    symbol_value = read_little_u64(symbol + 8);
                    return true;
                }
            }
        }
        return false;
    }

    bool load_init_elf(
        a9n::physical_address         initrd_start,
        a9n::physical_address         initrd_end,
        a9n::kernel::init_image_info &image_info
    )
    {
        if (!initrd_start || initrd_start >= initrd_end || initrd_end > DIRECT_MAP_END)
        {
            return false;
        }
        const auto  elf_size = initrd_end - initrd_start;
        const auto *elf      = physical_pointer<const uint8_t>(initrd_start);
        if (elf_size < 64 || elf[0] != 0x7f || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F'
            || elf[4] != 2 || elf[5] != 1 || read_little_u16(elf + 18) != ELF_MACHINE_AARCH64)
        {
            return false;
        }

        const auto program_offset     = read_little_u64(elf + 32);
        const auto program_entry_size = read_little_u16(elf + 54);
        const auto program_count      = read_little_u16(elf + 56);
        if (program_entry_size < 56
            || !valid_span(program_offset, static_cast<a9n::word>(program_entry_size) * program_count, elf_size))
        {
            return false;
        }

        a9n::word image_end = 0;
        for (uint16_t index = 0; index < program_count; ++index)
        {
            const auto *program = elf + program_offset + static_cast<size_t>(index) * program_entry_size;
            if (read_little_u32(program) != ELF_LOAD)
            {
                continue;
            }
            const auto physical = read_little_u64(program + 24);
            const auto memory   = read_little_u64(program + 40);
            if (physical > INIT_IMAGE_CAPACITY || memory > INIT_IMAGE_CAPACITY - physical)
            {
                return false;
            }
            if (physical + memory > image_end)
            {
                image_end = physical + memory;
            }
        }
        if (!image_end)
        {
            return false;
        }

        const auto image_pages = align_up(image_end) / a9n::PAGE_SIZE;
        clear_bytes(g_init_image, image_pages * a9n::PAGE_SIZE);
        for (uint16_t index = 0; index < program_count; ++index)
        {
            const auto *program = elf + program_offset + static_cast<size_t>(index) * program_entry_size;
            if (read_little_u32(program) != ELF_LOAD)
            {
                continue;
            }
            const auto file_offset = read_little_u64(program + 8);
            const auto physical    = read_little_u64(program + 24);
            const auto file_size   = read_little_u64(program + 32);
            const auto memory_size = read_little_u64(program + 40);
            if (file_size > memory_size || !valid_span(file_offset, file_size, elf_size))
            {
                return false;
            }
            copy_bytes(g_init_image + physical, elf + file_offset, file_size);
        }

        a9n::word init_info_address       = 0;
        a9n::word init_ipc_buffer_address = 0;
        if (!find_elf_symbol(elf, elf_size, "__init_info_start", init_info_address)
            || !find_elf_symbol(elf, elf_size, "__init_ipc_buffer_start", init_ipc_buffer_address))
        {
            return false;
        }

        image_info.loaded_address          = physical_address_of(g_init_image);
        image_info.init_image_size         = image_pages;
        image_info.entry_point_address     = read_little_u64(elf + 24);
        image_info.init_info_address       = init_info_address;
        image_info.init_ipc_buffer_address = init_ipc_buffer_address;
        return true;
    }

    void add_memory_map_entry(a9n::word start, a9n::word end, memory_map_type type)
    {
        start = align_up(start);
        end   = align_down(end);
        if (start >= end || g_memory_map_count >= MEMORY_MAP_CAPACITY)
        {
            return;
        }

        if (g_memory_map_count)
        {
            auto      &previous = g_memory_map[g_memory_map_count - 1];
            const auto previous_end
                = previous.start_physical_address + previous.page_count * a9n::PAGE_SIZE;
            if (previous.type == type && previous_end == start)
            {
                previous.page_count += (end - start) / a9n::PAGE_SIZE;
                return;
            }
        }

        g_memory_map[g_memory_map_count++] = memory_map_entry {
            .start_physical_address = start,
            .page_count             = (end - start) / a9n::PAGE_SIZE,
            .type                   = type,
        };
    }

    void construct_memory_map()
    {
        using enum memory_map_type;

        if (!g_ram_range_count)
        {
            add_range(g_ram_ranges, g_ram_range_count, 0x40000000, 0x80000000);
        }
        sort_and_merge_ranges(g_ram_ranges, g_ram_range_count);
        sort_and_merge_ranges(g_reserved_ranges, g_reserved_range_count);

        a9n::word previous_ram_end = 0;
        a9n::word memory_size      = 0;
        for (size_t ram_index = 0; ram_index < g_ram_range_count; ++ram_index)
        {
            const auto ram  = g_ram_ranges[ram_index];
            memory_size    += ram.end - ram.start;
            if (previous_ram_end < ram.start)
            {
                add_memory_map_entry(previous_ram_end, ram.start, DEVICE);
            }

            a9n::word cursor = ram.start;
            for (size_t reserved_index = 0; reserved_index < g_reserved_range_count; ++reserved_index)
            {
                const auto reserved = g_reserved_ranges[reserved_index];
                if (reserved.end <= ram.start || reserved.start >= ram.end)
                {
                    continue;
                }
                const auto start = reserved.start < ram.start ? ram.start : reserved.start;
                const auto end   = reserved.end > ram.end ? ram.end : reserved.end;
                if (cursor < start)
                {
                    add_memory_map_entry(cursor, start, FREE);
                }
                if (cursor < end)
                {
                    add_memory_map_entry(cursor > start ? cursor : start, end, RESERVED);
                    cursor = end;
                }
            }
            if (cursor < ram.end)
            {
                add_memory_map_entry(cursor, ram.end, FREE);
            }
            previous_ram_end = ram.end;
        }

        g_boot_info.boot_memory_info.memory_size      = memory_size;
        g_boot_info.boot_memory_info.memory_map_count = static_cast<uint16_t>(g_memory_map_count);
        g_boot_info.boot_memory_info.memory_map
            = reinterpret_cast<memory_map_entry *>(physical_address_of(g_memory_map));
    }
}

extern "C" a9n::physical_address
    aarch64_prepare_boot_protocol(a9n::physical_address initial_dtb_address)
{
    a9n::physical_address initrd_start = 0;
    a9n::physical_address initrd_end   = 0;
    a9n::word             dtb_size     = 0;

    g_boot_info.arch_info[0]           = initial_dtb_address;
    const bool valid_dtb = parse_fdt(initial_dtb_address, initrd_start, initrd_end, dtb_size);

    add_range(
        g_reserved_ranges,
        g_reserved_range_count,
        physical_address_of(__kernel_image_start_virtual),
        physical_address_of(__kernel_end_virtual)
    );
    if (valid_dtb)
    {
        add_range(
            g_reserved_ranges,
            g_reserved_range_count,
            initial_dtb_address,
            initial_dtb_address + dtb_size
        );
    }
    if (initrd_start && initrd_end > initrd_start)
    {
        add_range(g_reserved_ranges, g_reserved_range_count, initrd_start, initrd_end);
        load_init_elf(initrd_start, initrd_end, g_boot_info.boot_init_image_info);
    }

    construct_memory_map();
    return physical_address_of(&g_boot_info);
}
