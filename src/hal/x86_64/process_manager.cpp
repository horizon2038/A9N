#include "process_manager.hpp"
#include "common.hpp"
#include "process.hpp"

#include <library/string.hpp>
#include <library/logger.hpp>

#include "paging.hpp"

extern "C" void _switch_context(kernel::virtual_address *preview_stack_pointer, kernel::virtual_address *next_stack_pointer);

namespace hal::x86_64
{
    process_manager::process_manager()
        : _segment_configurator()
    {
    }

    process_manager::~process_manager()
    {
    }

    void process_manager::switch_context(kernel::process *preview_process, kernel::process *next_process)
    {
        _load_cr3(next_process->page_table);
        _segment_configurator.configure_rsp0(next_process->stack_pointer);
        _switch_context(&preview_process->stack_pointer, &next_process->stack_pointer);
    }

    void process_manager::create_process(kernel::process *target_process, kernel::virtual_address entry_point_address)
    {
        std::memset(&target_process->stack, 0, kernel::STACK_SIZE_MAX);
        kernel::virtual_address *target_stack_pointer = reinterpret_cast<kernel::virtual_address*>(&target_process->stack[kernel::STACK_SIZE_MAX]);

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

        *--target_stack_pointer = entry_point_address;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        *--target_stack_pointer = 0;
        // setup return address (program_counter) to stacks.

        target_process->stack_pointer = reinterpret_cast<kernel::virtual_address>(target_stack_pointer);

        kernel::utility::logger::printk("entry_point_address : %llx\n", entry_point_address);
        kernel::utility::logger::printk("in-stack entry_point_address : %llx\n", *target_stack_pointer);
        // target_process->stack_pointer = convert_virtual_to_physical_address(target_process->stack_pointer);
    }

    void process_manager::delete_process(kernel::process *target_process)
    {
    }

}
