#include <hal/x86_64/arch/arch_initializer.hpp>

#include <hal/x86_64/interrupt/pic.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/x86_64/arch/segment_configurator.hpp>

namespace hal::x86_64
{
    arch_initializer::arch_initializer()
    {
    }

    arch_initializer::~arch_initializer()
    {
    }

    void arch_initializer::init_architecture()
    {
        segment_configurator my_segment_configurator;
        my_segment_configurator.init_gdt();

        pic my_pic;
        my_pic.init_pic();
    }
}
