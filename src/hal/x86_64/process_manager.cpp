#include "process_manager.hpp"

extern void _switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer);

namespace hal::x86_64
{
    process_manager::process_manager()
    {
    }

    process_manager::~process_manager()
    {
    }

    void process_manager::switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer)
    {
    }

    void process_manager::switch_context(kernel::process *preview_process, kernel::process *next_process)
    {
        uint64_t *preview_stack_pointer = preview_process->stack_pointer;
        uint64_t *next_stack_pointer = next_process->stack_pointer;
        _switch_context(preview_stack_pointer, next_stack_pointer);
    }

}
