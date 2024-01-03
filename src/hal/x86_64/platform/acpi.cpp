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
}
