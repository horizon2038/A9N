#include "hal/x86_64/arch/arch_types.hpp"
#include <hal/x86_64/arch/arch_initializer.hpp>

#include <hal/x86_64/interrupt/pic.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/platform/acpi.hpp>

#include <kernel/boot/boot_info.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    arch_initializer::arch_initializer()
    {
    }

    arch_initializer::~arch_initializer()
    {
    }

    void arch_initializer::init_architecture(a9n::word arch_info[])
    {
        for (auto i = 0; i < 8; i++)
        {
            a9n::kernel::utility::logger::printk(
                "arch_info [%d] : 0x%016llx\n",
                i,
                arch_info[i]
            );
        }
        a9n::kernel::utility::logger::split();
        segment_configurator my_segment_configurator;
        my_segment_configurator.init_gdt();

        pic my_pic;
        my_pic.init_pic();

        acpi_configurator acpi;
        acpi.init_acpi(convert_physical_to_virtual_address(arch_info[0]));
    }
}
