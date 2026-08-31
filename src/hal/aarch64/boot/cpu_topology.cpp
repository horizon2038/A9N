#include <hal/aarch64/arch/cpu.hpp>

#include <hal/arch/arch_types.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/utility/logger.hpp>

#include <stddef.h>
#include <stdint.h>

namespace a9n::hal::aarch64
{
    namespace
    {
        constexpr uint32_t FDT_MAGIC      = 0xd00dfeed;
        constexpr uint32_t FDT_BEGIN_NODE = 1;
        constexpr uint32_t FDT_END_NODE   = 2;
        constexpr uint32_t FDT_PROPERTY   = 3;
        constexpr uint32_t FDT_NOP        = 4;
        constexpr uint32_t FDT_END        = 9;

        constexpr a9n::word MPIDR_AFFINITY_MASK = 0xff00ffffffULL;
        constexpr a9n::word DIRECT_MAP_END       = 16ULL * 1024 * 1024 * 1024;

        uint32_t read_big_u32(const uint8_t *source)
        {
            return static_cast<uint32_t>(source[0]) << 24
                 | static_cast<uint32_t>(source[1]) << 16
                 | static_cast<uint32_t>(source[2]) << 8
                 | static_cast<uint32_t>(source[3]);
        }

        bool valid_span(a9n::word offset, a9n::word size, a9n::word total)
        {
            return offset <= total && size <= total - offset;
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

        bool bounded_string_equals(
            const uint8_t *table,
            size_t         table_size,
            uint32_t       offset,
            const char    *name
        )
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

        bool property_string_equals(const uint8_t *data, uint32_t length, const char *name)
        {
            uint32_t index = 0;
            while (name[index])
            {
                if (index >= length || data[index] != static_cast<uint8_t>(name[index]))
                {
                    return false;
                }
                ++index;
            }
            return index == length || data[index] == 0;
        }

        a9n::word read_cells(const uint8_t *data, uint32_t cell_count)
        {
            if (!cell_count || cell_count > 2)
            {
                return 0;
            }
            a9n::word value = 0;
            for (uint32_t index = 0; index < cell_count; ++index)
            {
                value = value << 32 | read_big_u32(data + index * sizeof(uint32_t));
            }
            return value;
        }

        const char *enable_method_name(cpu_enable_method method)
        {
            switch (method)
            {
                case cpu_enable_method::PSCI :
                    return "psci";
                case cpu_enable_method::SPIN_TABLE :
                    return "spin-table";
                default :
                    return "none";
            }
        }

        const char *psci_conduit_name(psci_conduit conduit)
        {
            switch (conduit)
            {
                case psci_conduit::HVC :
                    return "hvc";
                case psci_conduit::SMC :
                    return "smc";
                default :
                    return "none";
            }
        }

        struct pending_cpu
        {
            cpu_description description {};
            bool            is_cpu { false };
            bool            has_mpidr { false };
            bool            enabled { true };
        };

        void append_cpu(const pending_cpu &cpu, a9n::word &count)
        {
            if (!cpu.is_cpu || !cpu.has_mpidr || !cpu.enabled
                || count >= a9n::kernel::CPU_COUNT_MAX)
            {
                return;
            }
            cpu_descriptions[count++] = cpu.description;
        }

        bool move_boot_cpu_first(a9n::word count)
        {
            a9n::word mpidr {};
            asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
            const auto boot_affinity = mpidr & MPIDR_AFFINITY_MASK;
            for (a9n::word index = 0; index < count; ++index)
            {
                if ((cpu_descriptions[index].mpidr & MPIDR_AFFINITY_MASK) != boot_affinity)
                {
                    continue;
                }
                const auto first        = cpu_descriptions[0];
                cpu_descriptions[0]     = cpu_descriptions[index];
                cpu_descriptions[index] = first;
                return true;
            }
            return false;
        }
    }

    hal_result discover_cpu_topology(a9n::physical_address dtb_address)
    {
        expected_core_count       = 1;
        discovered_psci_conduit  = psci_conduit::NONE;
        secondary_cores_runnable = 0;
        booted_core_count         = 1;
        secondary_boot_failed     = 0;

        if (!dtb_address || dtb_address >= DIRECT_MAP_END)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }
        const auto *fdt = a9n::kernel::physical_to_virtual_pointer<const uint8_t>(dtb_address);
        if (read_big_u32(fdt) != FDT_MAGIC)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        const uint32_t total_size       = read_big_u32(fdt + 4);
        const uint32_t structure_offset = read_big_u32(fdt + 8);
        const uint32_t strings_offset   = read_big_u32(fdt + 12);
        const uint32_t strings_size     = read_big_u32(fdt + 32);
        const uint32_t structure_size   = read_big_u32(fdt + 36);
        if (total_size < 40 || dtb_address > DIRECT_MAP_END - total_size
            || !valid_span(structure_offset, structure_size, total_size)
            || !valid_span(strings_offset, strings_size, total_size))
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 FDT: address=0x%016llx, size=%llu, structure=0x%08llx, strings=0x%08llx\n",
            dtb_address,
            static_cast<a9n::word>(total_size),
            static_cast<a9n::word>(structure_offset),
            static_cast<a9n::word>(strings_offset)
        );

        const auto    *structure        = fdt + structure_offset;
        const auto    *structure_end    = structure + structure_size;
        const auto    *strings          = fdt + strings_offset;
        const uint8_t *cursor           = structure;
        int            depth            = -1;
        bool           in_cpus          = false;
        bool           in_psci          = false;
        uint32_t       cpu_address_cells = 1;
        pending_cpu    current_cpu {};
        a9n::word      count = 0;

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
                    return hal_error::ILLEGAL_ARGUMENT;
                }
                ++cursor;
                cursor = reinterpret_cast<const uint8_t *>(
                    (reinterpret_cast<uintptr_t>(cursor) + 3) & ~static_cast<uintptr_t>(3)
                );
                ++depth;
                if (depth == 1)
                {
                    in_cpus = node_name_is(name, "cpus");
                    in_psci = node_name_is(name, "psci");
                }
                else if (depth == 2 && in_cpus)
                {
                    current_cpu        = {};
                    current_cpu.is_cpu = node_name_is(name, "cpu");
                }
                continue;
            }
            if (token == FDT_END_NODE)
            {
                if (depth == 2 && in_cpus)
                {
                    append_cpu(current_cpu, count);
                    current_cpu = {};
                }
                else if (depth == 1)
                {
                    in_cpus = false;
                    in_psci = false;
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
                return hal_error::ILLEGAL_ARGUMENT;
            }

            const uint32_t length      = read_big_u32(cursor);
            const uint32_t name_offset = read_big_u32(cursor + 4);
            cursor                    += 8;
            if (static_cast<size_t>(structure_end - cursor) < length || name_offset >= strings_size)
            {
                return hal_error::ILLEGAL_ARGUMENT;
            }
            const auto *data = cursor;

            if (depth == 1 && in_cpus && length == 4
                && bounded_string_equals(strings, strings_size, name_offset, "#address-cells"))
            {
                cpu_address_cells = read_big_u32(data);
            }
            else if (
                depth == 1 && in_psci
                && bounded_string_equals(strings, strings_size, name_offset, "method")
            )
            {
                if (property_string_equals(data, length, "hvc"))
                {
                    discovered_psci_conduit = psci_conduit::HVC;
                }
                else if (property_string_equals(data, length, "smc"))
                {
                    discovered_psci_conduit = psci_conduit::SMC;
                }
            }
            else if (
                depth == 2 && in_cpus
                && bounded_string_equals(strings, strings_size, name_offset, "device_type")
            )
            {
                current_cpu.is_cpu = property_string_equals(data, length, "cpu");
            }
            else if (
                depth == 2 && in_cpus
                && bounded_string_equals(strings, strings_size, name_offset, "status")
            )
            {
                current_cpu.enabled = property_string_equals(data, length, "okay")
                                   || property_string_equals(data, length, "ok");
            }
            else if (
                depth == 2 && in_cpus
                && bounded_string_equals(strings, strings_size, name_offset, "reg")
            )
            {
                if (cpu_address_cells >= 1 && cpu_address_cells <= 2
                    && length >= cpu_address_cells * sizeof(uint32_t))
                {
                    current_cpu.description.mpidr = read_cells(data, cpu_address_cells);
                    current_cpu.has_mpidr         = true;
                }
            }
            else if (
                depth == 2 && in_cpus
                && bounded_string_equals(strings, strings_size, name_offset, "enable-method")
            )
            {
                if (property_string_equals(data, length, "psci"))
                {
                    current_cpu.description.enable_method = cpu_enable_method::PSCI;
                }
                else if (property_string_equals(data, length, "spin-table"))
                {
                    current_cpu.description.enable_method = cpu_enable_method::SPIN_TABLE;
                }
            }
            else if (
                depth == 2 && in_cpus
                && bounded_string_equals(strings, strings_size, name_offset, "cpu-release-addr")
            )
            {
                const auto cells = length / sizeof(uint32_t);
                if (cells >= 1 && cells <= 2)
                {
                    current_cpu.description.release_address = read_cells(data, cells);
                }
            }

            cursor += length;
            cursor  = reinterpret_cast<const uint8_t *>(
                (reinterpret_cast<uintptr_t>(cursor) + 3) & ~static_cast<uintptr_t>(3)
            );
        }

        if (!count)
        {
            return hal_error::NO_SUCH_DEVICE;
        }
        if (!move_boot_cpu_first(count))
        {
            return hal_error::NO_SUCH_DEVICE;
        }
        expected_core_count = count;

        a9n::kernel::utility::logger::printh(
            "AArch64 CPU topology: cores=%llu, PSCI conduit=%s\n",
            expected_core_count,
            psci_conduit_name(discovered_psci_conduit)
        );
        for (a9n::word core = 0; core < expected_core_count; ++core)
        {
            a9n::kernel::utility::logger::printh(
                "  CPU %llu: MPIDR=0x%016llx, enable-method=%s, release-address=0x%016llx%s\n",
                core,
                cpu_descriptions[core].mpidr,
                enable_method_name(cpu_descriptions[core].enable_method),
                cpu_descriptions[core].release_address,
                core == 0 ? " (BSP)" : ""
            );
        }
        return {};
    }
}
