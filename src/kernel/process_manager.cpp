#include "process_manager.hpp"

namespace kernel
{
    process_manager::process_manager
    (
        hal::interface::process_manager &injected_process_manager,
        hal::interface::interrupt &injected_interrupt
    )
        : _process_manager(injected_process_manager)
        , _interrupt(injected_interrupt)
        , _scheduler()
    {
    }

    process_manager::~process_manager()
    {
    }

    void process_manager::create_process(const char *process_name, uint64_t process_address)
    {
    }

    void process_manager::delete_process(int32_t process_id)
    {
    }

    void process_manager::switch_context()
    {
        _interrupt.disable_interrupt_all();

        /*
        process *current_process;
        process *next_process;
        next_process = _scheduler.schedule_next_process();
        // undefined symnbol:
        _process_manager.switch_context(current_process, next_process);
        */

        _interrupt.enable_interrupt_all();
    }
}

