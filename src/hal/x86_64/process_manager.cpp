#include "process_manager.hpp"

extern "C" void _switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer);

namespace hal::x86_64
{
    process_manager::process_manager()
    {
    }

    process_manager::~process_manager()
    {
    }

    void process_manager::switch_context(kernel::process *preview_process, kernel::process *next_process)
    {
        uint64_t *preview_stack_pointer = preview_process->stack_pointer;
        uint64_t *next_stack_pointer = next_process->stack_pointer;
        _switch_context(preview_stack_pointer, next_stack_pointer);
    }

    void process_manager::init_process(kernel::process *target_process, uint64_t pc)
    {
        *target_process->stack_pointer-- = 0;
        *target_process->stack_pointer-- = 0;
        *target_process->stack_pointer-- = 0;
        *target_process->stack_pointer-- = 0;
        *target_process->stack_pointer-- = 0;
        *target_process->stack_pointer-- = 0;
        *target_process->stack_pointer-- = 0;
        // TODO: impl
        *target_process->stack_pointer-- = pc;
    }

    void process_manager::delete_process(kernel::process *target_process)
    {
    }

}
