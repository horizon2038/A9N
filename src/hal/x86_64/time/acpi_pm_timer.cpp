#include <hal/x86_64/platform/acpi.hpp>

#include <hal/x86_64/io/port_io.hpp>
#include <hal/x86_64/time/acpi_pm_timer.hpp>
#include <kernel/memory/memory.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{

    void dump_generic_address_structure(const generic_address_structure &generic);

    hal_result acpi_pm_timer::init(fadt *fadt_base)
    {
        using a9n::kernel::utility::logger;

        if (!fadt_base)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        if (fadt_base->pm_timer_length != 4)
        {
            return hal_error::ILLEGAL_DEVICE;
        }

        // configure ACPM PM Timer register bits
        bits = 24;
        if (fadt_base->flags & (1 << 8))
        {
            bits = 32;
        }

        // ACPI v1
        if (fadt_base->header.revision <= 2)
        {
            logger::printk("ACPI PM Timer : v1\n");
            is_mmio = false;
            port    = fadt_base->pm_timer_block;
            logger::printk("ACPI PM Timer : port : %016llx\n", port);
        }
        // ACPI v2+
        else
        {
            logger::printk("ACPI PM Timer : v2\n");
            using enum generic_address_space_type;
            switch (fadt_base->x_pm_timer_block.address_space)
            {
                case SYSTEM_MEMORY :
                    is_mmio = true;
                    address = a9n::kernel::physical_to_virtual_pointer<volatile uint32_t>(
                        fadt_base->x_pm_timer_block.address
                    );
                    logger::printk("ACPI PM Timer : address : %p\n", address);
                    break;

                case SYSTEM_IO :
                    is_mmio = false;
                    port    = fadt_base->x_pm_timer_block.address;
                    logger::printk("ACPI PM Timer : port : %016llx\n", port);
                    break;

                default :
                    return hal_error::ILLEGAL_DEVICE;
            }

            if (!port)
            {
                // fallback to acpi v1
                port = fadt_base->pm_timer_block;
            }
        }

        return {};
    }

    liba9n::result<uint32_t, hal_error> acpi_pm_timer::read()
    {
        using a9n::kernel::utility::logger;

        if (!address)
        {
            return hal_error::NO_SUCH_DEVICE;
        }

        uint32_t value = 0;
        if (is_mmio)
        {
            value = *address;
        }
        else
        {
            value = port_read_32(port & 0xFFFF);
        }

        const auto MASK_BITS = (bits < 32) ? ((1u << bits) - 1) : 0xFFFFFFFFu;
        return value & MASK_BITS;
    }

    hal_result acpi_pm_timer::wait(uint32_t micro_seconds)
    {
        using a9n::kernel::utility::logger;
        // logger::printk("ACPI PM Timer : wait %10d ms\n", micro_seconds / 1000);

        const auto MASK_BITS = (bits < 32) ? ((1u << bits) - 1) : 0xFFFFFFFFu;

        return read().and_then(
            [=, this](uint32_t start_time) -> hal_result
            {
                uint32_t end_ticks
                    = static_cast<uint32_t>(micro_seconds * (TIMER_FREQUENCY / 1e6)) & MASK_BITS;
                liba9n::result<uint32_t, hal_error> result { 0 };

                while (true)
                {
                    result = read();
                    if (!result)
                    {
                        a9n::kernel::utility::logger::printk(
                            "ACPI PM Timer : read "
                            "failed\n"
                        );
                        return result.unwrap_error();
                    }

                    auto     current_time = result.unwrap();
                    uint32_t elapsed_time = (current_time - start_time) & MASK_BITS;
                    if (elapsed_time >= end_ticks)
                    {
                        break;
                    }
                }

                return {};
            }
        );
    }

    void dump_generic_address_structure(const generic_address_structure &generic)
    {
        using a9n::kernel::utility::logger;

        logger::printk("dump generic_address_structure :\n");
        logger::printk("\e[48Gaddress_space : 0x%2x\n", generic.address_space);
        logger::printk("\e[48Gbit_width : 0x%2x\n", generic.bit_width);
        logger::printk("\e[48Gbit_offset : 0x%2x\n", generic.bit_offset);
        logger::printk("\e[48Gaccess_size : 0x%2x\n", generic.access_size);
        logger::printk("\e[48Gaddress : 0x%016x\n", generic.address);
    }

}
