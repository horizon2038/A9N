#include "arch_initializer.hpp"

#include <library/logger.hpp>
#include "segment_configurator.hpp"

namespace hal::x86_64
{
    void arch_initializer::init_architecture()
    {
        segment_configurator my_segment_configurator;
        my_segment_configurator.init_gdt();
    }
}
