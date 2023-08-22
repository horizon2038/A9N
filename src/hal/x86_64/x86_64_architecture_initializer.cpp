#include "x86_64_architecture_initializer.hpp"

namespace hal::x86_64
{

    architecture_initializer::architecture_initializer(segment_configurator &injected_segment_configurator) : _segment_configurator(injected_segment_configurator)
    {

    }

    architecture_initializer::~architecture_initializer()
    {

    }

    void architecture_initializer::init_architecture()
    {
        _segment_configurator.init_gdt();
    }
}

