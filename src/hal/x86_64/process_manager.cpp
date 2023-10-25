#include "process_manager.hpp"
#include "common.hpp"

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
        kernel::virtual_address *preview_stack_pointer = reinterpret_cast<kernel::virtual_address*>(preview_process->stack_pointer);
        kernel::virtual_address *next_stack_pointer = reinterpret_cast<kernel::virtual_address*>(next_process->stack_pointer);
        _switch_context(preview_stack_pointer, next_stack_pointer);
    }

    void process_manager::create_process(kernel::process *target_process, kernel::virtual_address entry_point_address)
    {
        /*
        init callee-saved register stacks.
        rbp
        rbx
        r12
        r13
        r14
        r15
        rflags
        */
        kernel::virtual_address *target_stack_pointer = reinterpret_cast<kernel::virtual_address*>(target_process->stack_pointer);
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        // setup return address (program_counter) to stacks.
        *target_stack_pointer-- = entry_point_address;
    }

    void process_manager::delete_process(kernel::process *target_process)
    {
    }

}
