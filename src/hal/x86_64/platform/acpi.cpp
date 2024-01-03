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
            return;
        }
        kernel::utility::logger::printk("RSDP is valid\n");
    };

    bool acpi_configurator::validate_rsdp(common::virtual_address rsdp_address)
    {
        kernel::utility::logger::printk("validate_rsdp\n");
        rsdp *rsdp_pointer = reinterpret_cast<rsdp *>(rsdp_address);
        return (std::memcmp(rsdp_pointer->signature, ACPI_MAGIC::RSDP, 8) == 0);
    }
}
