#include "context_switch.hpp"
#include "interface/process_manager.hpp"

namespace kernel
{
    context_switch::context_switch
    (
        hal::interface::process_manager &injected_process_manager,
        hal::interface::interrupt &injected_interrupt
    )
        : _process_manager(injected_process_manager)
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

        _process_manager.switch_context(preview_process, next_process);

        _interrupt.enable_interrupt_all();
    }
}
