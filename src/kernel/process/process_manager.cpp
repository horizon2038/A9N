#include <kernel/process/process_manager.hpp>

#include <hal/interface/process_manager.hpp>
#include <kernel/kernel.hpp>
#include <kernel/utility/logger.hpp>
#include <kernel/process/process.hpp>

#include <stdint.h>
#include <liba9n/libc/string.hpp>

namespace a9n::kernel
{
    process_manager::process_manager(
        a9n::hal::process_manager &target_process_manager
    )
        : _scheduler(process_list)
        , _process_manager(target_process_manager)
    {
        current_process = &process_list[0];

        highest_priority = 0;
    }

    process_manager::~process_manager()
    {
    }

    void process_manager::create_process(
        const char          *process_name,
        a9n::virtual_address entry_point_address
    )
    {
        uint16_t process_id = determine_process_id();

        if (process_id <= 0)
        {
            return; // impl error
        }

        process *current_process = &process_list[process_id];
        init_process(
            current_process,
            process_id,
            process_name,
            entry_point_address
        );
        _process_manager.create_process(current_process, entry_point_address);
        current_process->status = process_status::READY;

        utility::logger::printk("create process\n");
        utility::logger::printk(
            "id : %d | name : %s | status : %d | page_table_address : "
            "0x%016llx | stack_pointer : 0x%016llx\n",
            current_process->id,
            current_process->name,
            current_process->status,
            current_process->page_table,
            current_process->stack_pointer
        );
        utility::logger::split();

        // test priority-scheduling
        a9n::sword priority = current_process->priority;
        if (priority_groups[priority] == nullptr)
        {
            priority_groups[priority] = current_process;
            return;
        }

        process *temp_process = priority_groups[priority];
        while (temp_process->next != nullptr)
        {
            temp_process = temp_process->next;
        }
        temp_process->next       = current_process;
        current_process->preview = temp_process;
    }

    void process_manager::init_process(
        process             *process,
        process_id           target_process_id,
        const char          *process_name,
        a9n::virtual_address entry_point_address
    )
    {
        process->id = target_process_id;
        liba9n::std::strcpy(process->name, process_name);

        process->status   = process_status::BLOCKED;
        process->priority = 0;
        process->quantum  = QUANTUM_MAX;

        liba9n::std::memset(process->stack, 0, STACK_SIZE_MAX);
        kernel_object::memory_manager->init_virtual_memory(process);
    }

    a9n::sword process_manager::determine_process_id()
    {
        // 0 == init_server id (reserved).

        for (uint16_t i = 1; i < PROCESS_COUNT_MAX; i++)
        {
            if (process_list[i].status != process_status::UNUSED)
            {
                continue;
            }
            return i;
        }

        return -1;
    }

    void process_manager::delete_process(process_id target_process_id)
    {
    }

    void process_manager::switch_context()
    {
        process *temp_current_process = current_process;
        process *next_process         = _scheduler.schedule_next_process(
            priority_groups,
            highest_priority
        );
        if (next_process == nullptr)
        {
            return;
        }

        current_process = next_process;
        _process_manager.switch_context(temp_current_process, next_process);
    }

    process *
        process_manager::search_process_from_id(process_id target_process_id)
    {
        if (target_process_id <= 0)
        {
            return nullptr;
        }

        return &process_list[target_process_id];
    }
}
