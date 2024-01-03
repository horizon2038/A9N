#include "library/common/types.hpp"
#include <hal/x86_64/platform/acpi.hpp>

#include <hal/x86_64/arch/arch_types.hpp>
#include <library/libc/string.hpp>
#include <kernel/utility/logger.hpp>

namespace hal::x86_64
{
    acpi_configurator::acpi_configurator() {};
    acpi_configurator::~acpi_configurator() {};

    void acpi_configurator::init_acpi(common::virtual_address rsdp_address)
    {
        if (!validate_rsdp(rsdp_address))
        {
            kernel::utility::logger::printk("RSDP is invalid\n");
            return;
        }
        kernel::utility::logger::printk("RSDP is valid\n");

        auto rsdp_pointer = reinterpret_cast<rsdp *>(rsdp_address);
        print_rsdp_info(rsdp_pointer);
    };

    bool acpi_configurator::validate_rsdp(common::virtual_address rsdp_address)
    {
        rsdp *rsdp_pointer = reinterpret_cast<rsdp *>(rsdp_address);
        return (std::memcmp(rsdp_pointer->signature, ACPI_MAGIC::RSDP, 8) == 0);
    }

    void acpi_configurator::print_rsdp_info(rsdp *rsdp_pointer)
    {
        char signature[9];
        signature[8] = '\0';

        char oem_id[7];
        oem_id[6] = '\0';

        std::memcpy(signature, rsdp_pointer->signature, 8);
        std::memcpy(oem_id, rsdp_pointer->oem_id, 6);

        kernel::utility::logger::printk("signature\e[50G : %s\n", signature);
        kernel::utility::logger::printk(
            "checksum\e[50G : %d\n",
            rsdp_pointer->checksum
        );
        kernel::utility::logger::printk("oem_id\e[50G : %s\n", oem_id);
        kernel::utility::logger::printk(
            "revision\e[50G : %d\n",
            rsdp_pointer->revision
        );
        kernel::utility::logger::printk(
            "rsdt_address\e[50G : 0x%08lx\n",
            rsdp_pointer->rsdt_address
        );
        kernel::utility::logger::printk(
            "length\e[50G : 0x%08lx\n",
            rsdp_pointer->length
        );
        kernel::utility::logger::printk(
            "xsdt_address\e[50G : 0x%016llx\n",
            rsdp_pointer->xsdt_address
        );
        kernel::utility::logger::printk(
            "extended_checksum\e[50G : %d\n",
            rsdp_pointer->extended_checksum
        );
    }
}
