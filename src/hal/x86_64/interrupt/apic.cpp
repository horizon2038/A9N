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
        a9n::kernel::utility::logger::printh("configure from MADT\n");
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
                    }

                default :
                    break;
            }

            madt_entry_pointer += entry->length;
        }

        a9n::kernel::utility::logger::printh("IO APIC : id %4llx\n", id);
        a9n::kernel::utility::logger::printh("IO APIC : base address 0x%016llx\n", base_address);
        a9n::kernel::utility::logger::printh(
            "IO APIC : global interrupt base 0x%016llx\n",
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
        a9n::kernel::utility::logger::printh("IO APIC : disable_interrupt\n");
        return read(io_apic_register_index::VERSION)
            .and_then(
                [this](uint32_t version) -> hal_result
                {
                    uint8_t max_redirection_entries = ((version >> 16) & 0xFF) + 1;

                    hal_result result {};
                    for (uint8_t irq_number = 0; irq_number < max_redirection_entries; irq_number++)
                    {
                        a9n::kernel::utility::logger::printh("IO APIC : disable IRQ %4d\n", irq_number);
                        result = disable_interrupt(irq_number);
                        if (!result)
                        {
                            a9n::kernel::utility::logger::printh(
                                "IO APIC : disable IRQ %4d failed\n",
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
        a9n::kernel::utility::logger::printh("IO APIC : enable_interrupt\n");
        return read(io_apic_register_index::VERSION)
            .and_then(
                [this](uint32_t version) -> hal_result
                {
                    uint8_t max_redirection_entries = ((version >> 16) & 0xFF) + 1;

                    hal_result result {};
                    for (uint8_t irq_number = 0; irq_number < max_redirection_entries; irq_number++)
                    {
                        a9n::kernel::utility::logger::printh("IO APIC : enable IRQ %4d\n", irq_number);
                        result = enable_interrupt(irq_number);
                        if (!result)
                        {
                            a9n::kernel::utility::logger::printh(
                                "IO APIC : enable IRQ %4d failed\n",
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
        const uint8_t real_irq_number  = global_interrupt_base + irq_number;
        uint32_t      io_apic_register = io_apic_register_index::REDIRECTION_TABLE + irq_number * 2;

        // low
        return write(
                   io_apic_register,
                   (liba9n::enum_cast(reserved_irq::IO_BASE) + real_irq_number) | (1 << 16)
        )
            .and_then(
                [=, this](void) -> hal_result
                {
                    // high
                    // redirect to BSP
                    return write(io_apic_register + 1, 0);
                }
            );
    }

    hal_result io_apic::enable_interrupt(uint8_t irq_number)
    {
        // FIXME: real_irq_number -> irq_number
        const uint8_t  real_irq_number = global_interrupt_base + irq_number;
        const uint32_t io_apic_register
            = io_apic_register_index::REDIRECTION_TABLE + real_irq_number * 2;

        // low
        return write(io_apic_register, liba9n::enum_cast(reserved_irq::IO_BASE) + real_irq_number)
            .and_then(
                [=, this](void) -> hal_result
                {
                    // high
                    // redirect to BSP
                    return write(io_apic_register + 1, 0);
                }
            );
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

        logger::printh("APIC base address : 0x%016llx\n", apic_base_address);

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
