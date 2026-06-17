#include <hal/x86_64/interrupt/apic.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>
#include <hal/x86_64/interrupt/pic.hpp>
#include <hal/x86_64/platform/acpi.hpp>

#include <kernel/memory/memory.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/common/enum.hpp>

namespace a9n::hal::x86_64
{
    // IO APIC
    hal_result io_apic::init(madt *madt_base)
    {
        return configure_from_madt(madt_base)
            .and_then(
                [this](void) -> hal_result
                {
                    return configure_registers();
                }
            )
            .and_then(
                [this](void) -> hal_result
                {
                    return disable_interrupt_all().and_then(
                        [this](void) -> hal_result
                        {
                            return local_apic_core.end_of_interrupt();
                        }
                    );

                    return {};
                }
            );
    }

    hal_result io_apic::configure_from_madt(madt *madt_base)
    {
        a9n::kernel::utility::logger::printh("Configuring IO APIC from MADT: address=%p\n", madt_base);
        if (!madt_base)
        {
            a9n::kernel::utility::logger::printh("MADT is empty\n");
            return hal_error::ILLEGAL_ARGUMENT;
        }

        uint8_t *madt_entry_pointer = reinterpret_cast<uint8_t *>(madt_base) + sizeof(madt);
        uint8_t *end = reinterpret_cast<uint8_t *>(madt_base) + madt_base->header.length;

        while (madt_entry_pointer < end)
        {
            madt_entry_header *entry = reinterpret_cast<madt_entry_header *>(madt_entry_pointer);

            switch (entry->type)
            {
                case madt_entry_type::IO_APIC :
                    {
                        madt_io_apic *io_apic_entry = reinterpret_cast<madt_io_apic *>(entry);

                        id                          = io_apic_entry->io_apic_id;
                        base_address                = io_apic_entry->io_apic_address;
                        global_interrupt_base       = io_apic_entry->global_system_interrupt_base;
                        break;
                    }
                case madt_entry_type::INTERRUPT_SOURCE_OVERRIDE :
                    {
                        madt_interrupt_source_override *override_entry
                            = reinterpret_cast<madt_interrupt_source_override *>(entry);

                        if (override_entry->irq_source < 16)
                        {
                            interrupt_source_overrides[override_entry->irq_source].valid = true;
                            interrupt_source_overrides[override_entry->irq_source].irq_source
                                = override_entry->irq_source;
                            interrupt_source_overrides[override_entry->irq_source].global_system_interrupt
                                = override_entry->global_system_interrupt;
                            interrupt_source_overrides[override_entry->irq_source].flags
                                = override_entry->flags;

                            a9n::kernel::utility::logger::printh(
                                "  MADT ISO: IRQ=%3d -> GSI=%3d, flags=0x%04x\n",
                                override_entry->irq_source,
                                override_entry->global_system_interrupt,
                                override_entry->flags
                            );
                        }
                        break;
                    }

                default :
                    break;
            }

            madt_entry_pointer += entry->length;
        }

        a9n::kernel::utility::logger::printh(
            "  ID=%4llx, address=%p, global_interrupt_base=0x%08llx\n",
            id,
            base_address,
            global_interrupt_base
        );

        return {};
    }

    hal_result io_apic::configure_registers(void)
    {
        uint8_t *io_apic_base = a9n::kernel::physical_to_virtual_pointer<uint8_t>(base_address);
        if (!io_apic_base)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        register_select
            = reinterpret_cast<volatile uint32_t *>(io_apic_base + io_apic_offset::REGISTER_SELECT);
        window = reinterpret_cast<volatile uint32_t *>(io_apic_base + io_apic_offset::REGISTER_WINDOW);

        return {};
    }

    hal_result io_apic::disable_interrupt_all(void)
    {
        a9n::kernel::utility::logger::printh("IO APIC: disable_interrupt\n");
        return read(io_apic_register_index::VERSION)
            .and_then(
                [this](uint32_t version) -> hal_result
                {
                    uint8_t max_redirection_entries = ((version >> 16) & 0xFF) + 1;

                    hal_result result {};
                    for (uint8_t irq_number = 0; irq_number < max_redirection_entries; irq_number++)
                    {
                        a9n::kernel::utility::logger::printh("IO APIC: disable IRQ %4d\n", irq_number);
                        result = disable_interrupt(irq_number);
                        if (!result)
                        {
                            a9n::kernel::utility::logger::printh(
                                "IO APIC: disable IRQ %4d failed\n",
                                irq_number
                            );
                            break;
                        }
                    }

                    return result;
                }
            );
    }

    hal_result io_apic::enable_interrupt_all(void)
    {
        a9n::kernel::utility::logger::printh("IO APIC: enable_interrupt\n");
        return read(io_apic_register_index::VERSION)
            .and_then(
                [this](uint32_t version) -> hal_result
                {
                    uint8_t max_redirection_entries = ((version >> 16) & 0xFF) + 1;

                    hal_result result {};
                    for (uint8_t irq_number = 0; irq_number < max_redirection_entries; irq_number++)
                    {
                        a9n::kernel::utility::logger::printh("IO APIC: enable IRQ %4d\n", irq_number);
                        result = enable_interrupt(irq_number);
                        if (!result)
                        {
                            a9n::kernel::utility::logger::printh(
                                "IO APIC: enable IRQ %4d failed\n",
                                irq_number
                            );
                            break;
                        }
                    }

                    return result;
                }
            );
    }

    hal_result io_apic::configure_entry(uint8_t irq_number, uint64_t data)
    {
        uint32_t io_apic_register = io_apic_register_index::REDIRECTION_TABLE + irq_number * 2;

        // low
        return write(io_apic_register, static_cast<uint32_t>(data & 0xFFFFFFFF))
            .and_then(
                [=, this](void) -> hal_result
                {
                    // high
                    return write(io_apic_register + 1, static_cast<uint32_t>((data >> 32) & 0xFFFFFFFF));
                }
            );
    }

    hal_result io_apic::disable_interrupt(uint8_t irq_number)
    {
        return configure_irq(irq_number, MASKED);
    }

    hal_result io_apic::enable_interrupt(uint8_t irq_number)
    {
        return configure_irq(irq_number, UNMASKED);
    }

    hal_result io_apic::configure_irq(uint8_t irq_number, bool mask)
    {
        const uint32_t global_system_interrupt = resolve_global_system_interrupt(irq_number);

        if (global_system_interrupt < global_interrupt_base)
        {
            a9n::kernel::utility::logger::printh(
                "IO APIC: IRQ %2x -> GSI %2x < GSI base %2x\n",
                irq_number,
                global_system_interrupt,
                global_interrupt_base
            );
            return hal_error::ILLEGAL_ARGUMENT;
        }

        const uint32_t io_apic_pin   = global_system_interrupt - global_interrupt_base;
        const uint16_t flags         = resolve_interrupt_flags(irq_number);

        PIN_POLARITY pin_polarity    = ACTIVE_HIGH;
        TRIGGER_MODE trigger_mode    = EDGE;

        const uint16_t polarity_bits = flags & 0x3;
        const uint16_t trigger_bits  = (flags >> 2) & 0x3;

        // ACPI MADT flags
        // polarity:
        //   00 = conforms
        //   01 = active high
        //   11 = active low
        // trigger:
        //   00 = conforms
        //   01 = edge
        //   11 = level
        if (polarity_bits == 0x3)
        {
            pin_polarity = ACTIVE_LOW;
        }

        if (trigger_bits == 0x3)
        {
            trigger_mode = LEVEL;
        }

        const uint8_t vector     = liba9n::enum_cast(reserved_irq::IO_BASE) + irq_number;
        const MASK    mask_typed = mask ? MASKED : UNMASKED;

        const uint64_t entry
            = make_redirect_entry(vector, FIXED, PHYSICAL, IDLE, pin_polarity, trigger_mode, mask_typed, 0);

        return configure_entry(static_cast<uint8_t>(io_apic_pin), entry);
    }

    uint32_t io_apic::resolve_global_system_interrupt(uint8_t irq_number) const
    {
        if (irq_number < 16)
        {
            if (interrupt_source_overrides[irq_number].valid)
            {
                return interrupt_source_overrides[irq_number].global_system_interrupt;
            }
        }

        return irq_number;
    }

    uint16_t io_apic::resolve_interrupt_flags(uint8_t irq_number) const
    {
        if (irq_number < 16)
        {
            if (interrupt_source_overrides[irq_number].valid)
            {
                return interrupt_source_overrides[irq_number].flags;
            }
        }

        // Heuristic default for PCI INTx routes on PC-compatible machines:
        // when MADT has no explicit ISO for GSI16..23, treat them as
        // Active-Low + Level-Triggered.
        if (irq_number >= 16 && irq_number <= 23)
        {
            return 0x0f;
        }

        return 0;
    }

    liba9n::result<uint32_t, hal_error> io_apic::read(uint32_t io_apic_register)
    {
        if (!register_select || !window)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        *register_select = io_apic_register;
        return static_cast<uint32_t>(*window);
    }

    hal_result io_apic::write(uint32_t io_apic_register, uint32_t value)
    {
        if (!register_select || !window)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        *register_select = io_apic_register;
        *window          = value;

        return {};
    }

    // Local APIC
    hal_result local_apic::init(void)
    {
        using a9n::kernel::utility::logger;

        uint64_t              apic_base_msr     = _read_msr(msr::APIC_BASE);
        a9n::physical_address apic_base_address = apic_base_msr & 0xFFFF'1000;

        base = a9n::kernel::physical_to_virtual_pointer<uint32_t>(apic_base_address);
        if (!base)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        logger::printh("APIC base address: 0x%016llx\n", apic_base_address);

        // enable APIC
        apic_base_address |= local_apic_flag::APIC_ENABLE;
        _write_msr(msr::APIC_BASE, apic_base_address);

        auto write_register = [this](uint32_t offset, uint64_t value) -> decltype(auto)
        {
            return [this, offset, value](void) -> hal_result
            {
                return write(offset, value);
            };
        };

        // configure spurious
        return write(local_apic_offset::SPURIOUS_INTERRUPT, (1 << 8) | ((1 << 4) - 1))
            .and_then(write_register(local_apic_offset::TASK_PRIORITY, 0x0))
            .and_then(write_register(local_apic_offset::LOGICAL_DESITINATION, 0x100'0000))
            .and_then(write_register(local_apic_offset::DESTINATION_FORMAT, 0xFFFF'FFFF))
            .and_then(write_register(local_apic_offset::LVT_TIMER, 1 << 16))
            .and_then(write_register(local_apic_offset::LVT_PERFORMANCE_COUNTER, 1 << 16))
            .and_then(write_register(local_apic_offset::LVT_LINT_0, 0x8700))
            .and_then(write_register(local_apic_offset::LVT_LINT_1, 0x400))
            .and_then(write_register(local_apic_offset::LVT_ERROR, 1 << 16))
            .and_then(write_register(local_apic_offset::END_OF_INTERRUPT, 0));
    }

    liba9n::result<uint32_t, hal_error> local_apic::read(uint32_t offset)
    {
        if (!base)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return static_cast<uint32_t>(base[offset / 4]);
    }

    hal_result local_apic::write(uint32_t offset, uint64_t value)
    {
        if (!base)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        base[offset / 4] = value;

        return {};
    }

    hal_result local_apic::end_of_interrupt(void)
    {
        return write(local_apic_offset::END_OF_INTERRUPT, 0);
    }
}
