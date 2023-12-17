#include "scheduler.hpp"
#include "common.hpp"

#include <library/logger.hpp>

namespace kernel
{
    scheduler::scheduler(process *process_list)
    : _process_list(process_list)
    , _current_process(nullptr)
    {
        current_process_index = 0;
    }
    
    scheduler::~scheduler()
    {
    }

    process *scheduler::schedule_next_process()
    {
        process *target_process = nullptr;
        uint32_t start_index = current_process_index;

        do
        {
            if (current_process_index >= 12)
            {
                current_process_index = 0;
            }

            target_process = &_process_list[current_process_index];

            utility::logger::printk("cpi : %llu\n", current_process_index);
            utility::logger::printk("target_process_status : %llu\n", target_process->status);
            current_process_index++;

        }
        while(target_process->status != process_status::READY && current_process_index != start_index);  

        if (target_process->status != process_status::READY)
        {
            utility::logger::printk("can't find schedulable process\n");
            return nullptr;
        }

        if (_current_process != nullptr)
        {
            _current_process->status = process_status::READY;
        }

        target_process->status = process_status::RUNNING;
        _current_process = target_process;

        return target_process;
    }

    process *scheduler::schedule_next_process(process *priority_groups[], int32_t &highest_priority)
    {
        if (highest_priority == -1)
        {
            return nullptr;
        }

        process *current_process = priority_groups[highest_priority];

        current_process->quantum--;
        // utility::logger::printk("%s quantum : [ %04d ]\n", current_process->name, current_process->quantum);
        if (current_process->quantum == 0 || current_process->status == process_status::BLOCKED)
        {
            // utility::logger::printk("%s : %016s [ %04d ]\n", current_process->name, current_process->quantum == 0 ? "TIMEOUT" : "BLOCKED", current_process->quantum);
            current_process->quantum = QUANTUM_MAX;
            move_to_end(current_process, priority_groups[highest_priority]);
        }

        highest_priority = update_highest_priority(priority_groups);

        if (priority_groups[highest_priority] == nullptr)
        {
            return nullptr;
        }

        if (_current_process != nullptr)
        {
            _current_process->status = process_status::READY;
        }

        priority_groups[highest_priority]->status = process_status::RUNNING;
        _current_process = priority_groups[highest_priority];

        // utility::logger::printk("target_process_status : %llu\n", priority_groups[highest_priority]->status);
        return priority_groups[highest_priority];
    }

    void scheduler::move_to_end(process *target_process, process *&head_process)
    {
        if (target_process->next == nullptr)
        {
            utility::logger::printk("Target process is already at the end\n");
            return;
        }

        if (head_process == target_process)
        {
            head_process = target_process->next;
            head_process->preview = nullptr; 
        }

        if (target_process->preview != nullptr)
        {
            target_process->preview->next = target_process->next;
        }

        target_process->next->preview = target_process->preview;

        process *temp_process = head_process;

        while (temp_process->next != nullptr)
        {
            temp_process = temp_process->next;
        }

        temp_process->next = target_process;
        target_process->preview = temp_process;
        target_process->next = nullptr;
    }

    int32_t scheduler::update_highest_priority(process *priority_groups[])
    {
        for (uint16_t i = (PRIORITY_MAX - 1); i >= 0; i--)
        {
            if (priority_groups[i] != nullptr)
            {
                return i;
            }
        }

        return -1;
    }
}
