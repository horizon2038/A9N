#include "library/common/types.hpp"
#include <hal/x86_64/platform/acpi.hpp>

#include <hal/x86_64/arch/arch_types.hpp>
#include <library/libc/string.hpp>
#include <kernel/utility/logger.hpp>

namespace hal::x86_64
{
    acpi_configurator::acpi_configurator() {};
    acpi_configurator::~acpi_configurator() {};

    void acpi_configurator::init_acpi() {};

    common::virtual_address acpi_configurator::find_rsdp()
    {
        common::virtual_address ebda_base
            = convert_physical_to_virtual_address(ACPI_REGION::EBDA_SEGMENT);
        kernel::utility::logger::printk("ebda_base : 0x%016llx\n", ebda_base);

        common::virtual_address ebda_address
            = *reinterpret_cast<uint16_t *>(ebda_base) << 4;
        kernel::utility::logger::printk(
            "ebda_address : 0x%016llx\n",
            ebda_address
        );

        common::virtual_address bios_main_address
            = convert_physical_to_virtual_address(ACPI_REGION::BIOS_MAIN_START);

        return find_rsdp_range(ebda_address, ebda_address + 0x20000)
            || find_rsdp_range(bios_main_address, bios_main_address + 0x20000);
    };

    common::virtual_pointer acpi_configurator::find_rsdp_range(
        common::virtual_address start_address,
        common::virtual_address end_address
    )
    {
        uint8_t *address;
        for (address = reinterpret_cast<uint8_t *>(start_address);
             address <= reinterpret_cast<uint8_t *>(end_address);
             address += 0x10)
        {
            if (std::memcmp(address, ACPI_MAGIC::RSDP, 8) == 0)
            {
                kernel::utility::logger::printk(
                    "RSDP found at address: 0x%016llx\n",
                    address
                );
                return reinterpret_cast<common::virtual_pointer>(address);
            }
        }
        kernel::utility::logger::printk(
            "RSDP not found in the specified range.\n"
        );
        return reinterpret_cast<common::virtual_address>(nullptr);
    }
}
