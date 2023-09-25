#include "context_switch.hpp"

namespace kernel
{
    context_switch::context_switch
    (
        hal::interface::context_switch &injected_context_switch,
        hal::interface::interrupt &injected_interrupt
    )
        : _context_switch(injected_context_switch)
        , _interrupt(injected_interrupt)
    {
    }
    
    context_switch::~context_switch()
    {
    }

    void context_switch::switch_context(process *preview_process, process *next_process)
    {
        // hardware specific processing (hardware-dependent)
        _interrupt.disable_interrupt_all();

        _context_switch.switch_context(preview_process, next_process);

        _interrupt.enable_interrupt_all();
    }
}
