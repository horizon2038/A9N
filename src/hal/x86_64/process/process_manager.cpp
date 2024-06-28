#include <hal/x86_64/process/process_manager.hpp>

#include <hal/x86_64/memory/paging.hpp>

#include <kernel/process/process.hpp>
#include <kernel/utility/logger.hpp>

#include <library/libc/string.hpp>

extern "C" void _switch_context(
    a9n::virtual_address *preview_stack_pointer,
    a9n::virtual_address *next_stack_pointer
);

namespace a9n::hal::x86_64
{
    process_manager::process_manager() : _segment_configurator()
    {
    }

    process_manager::~process_manager()
    {
    }

    void process_manager::switch_context(
        a9n::kernel::process *preview_process,
        a9n::kernel::process *next_process
    )
    {
        _load_cr3(next_process->page_table);
        _segment_configurator.configure_rsp0(next_process->stack_pointer);
        _switch_context(
            &preview_process->stack_pointer,
            &next_process->stack_pointer
        );
    }

    void process_manager::create_process(
        a9n::kernel::process *target_process,
        a9n::virtual_address  entry_point_address
    )
    {
        liba9n::std::memset(
            &target_process->stack,
            0,
            a9n::kernel::STACK_SIZE_MAX
        );
        a9n::virtual_address *target_stack_pointer
            = reinterpret_cast<a9n::virtual_address *>(
                &target_process->stack[a9n::kernel::STACK_SIZE_MAX]
            );

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

        target_process->stack_pointer
            = reinterpret_cast<a9n::virtual_address>(target_stack_pointer);

        a9n::kernel::utility::logger::printk(
            "entry_point_address : %llx\n",
            entry_point_address
        );
        // target_process->stack_pointer =
        // convert_virtual_to_physical_address(target_process->stack_pointer);
    }

    void process_manager::delete_process(a9n::kernel::process *target_process)
    {
    }

}
