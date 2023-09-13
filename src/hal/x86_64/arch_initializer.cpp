#include "arch_initializer.hpp"

namespace hal::x86_64
{
    arch_initializer::arch_initializer
    (
        segment_configurator &injected_segment_configurator
    )
    : _segment_configurator(injected_segment_configurator)
    {

    }

    arch_initializer::~arch_initializer()
    {

    }

    void arch_initializer::init_architecture()
    {
        _segment_configurator.init_gdt();
    }
}

