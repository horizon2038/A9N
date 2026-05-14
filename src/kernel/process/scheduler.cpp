#include <kernel/process/scheduler.hpp>

#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/result/result.hpp>

namespace a9n::kernel
{
    // simple benno scheduler (round-robin with priority)
    // the preview pointer may seem unnecessary, but it is necessary for direct-switch
    liba9n::result<process *, scheduler_error> scheduler::schedule(void)
    {
        using a9n::kernel::utility::logger;
        process *next_process {};

        for (a9n::sword i = highest_priority; i >= 0; i--)
        {
            auto head = queue[i].head;
            if (!head)
            {
                continue;
            }

            // get process
            next_process = head;

            // remove process
            queue[i].head = next_process->next;
            if (queue[i].head)
            {
                queue[i].head->preview = nullptr;
            }
            else
            {
                // queue is empty now
                queue[i].tail = nullptr;
            }

            // update status
            highest_priority      = next_process->priority;

            next_process->next    = nullptr;
            next_process->preview = nullptr;

            return next_process;
        }

        // no executable process exists (basically the caller would run IDLE)
        return scheduler_error::PROCESS_NOT_IN_QUEUE;
    }

    // direct context switch to accelerate ipc
    liba9n::result<process *, scheduler_error> scheduler::try_direct_schedule(process *target_process)
    {
        if (!target_process)
        {
            DEBUG_LOG("invalid process");
            return scheduler_error::INVALID_PROCESS;
        }

        auto target_priority = target_process->priority;
        if (target_priority < 0 || target_priority >= PRIORITY_MAX) [[unlikely]]
        {
            DEBUG_LOG("invalid priority");
            return scheduler_error::INVALID_PRIORITY;
        }
        if (target_priority < highest_priority) [[unlikely]]
        {
            if (target_process->status != process_status::READY)
            {
                DEBUG_LOG("invalid process status");
                return scheduler_error::INVALID_PROCESS;
            }

            auto add_result = add_process(target_process);
            if (!add_result)
            {
                return add_result.unwrap_error();
            }

            return schedule();
        }

        auto &target_queue = queue[target_priority];
        bool  is_head      = (target_queue.head == target_process);
        bool  is_tail      = (target_queue.tail == target_process);
        bool  has_next     = static_cast<bool>(target_process->next);
        bool  has_preview  = static_cast<bool>(target_process->preview);

        // Unlink target process if it is in the ready-queue.
        // (single-element queue case: head==tail==target, next==preview==nullptr)
        if (is_head || is_tail || has_next || has_preview)
        {
            auto preview = target_process->preview;
            auto next    = target_process->next;

            if (preview)
            {
                preview->next = next;
            }
            if (next)
            {
                next->preview = preview;
            }
            if (is_head)
            {
                target_queue.head = next;
            }
            if (is_tail)
            {
                target_queue.tail = preview;
            }

            if (!target_queue.head)
            {
                target_queue.tail = nullptr;
            }
            else if (!target_queue.tail)
            {
                target_queue.tail = target_queue.head;
            }
        }
        // if target_process does not exist in queue, it can be scheduled as it is

        // update status
        highest_priority        = target_process->priority;

        target_process->next    = nullptr;
        target_process->preview = nullptr;

        return target_process;
    }

    scheduler_result scheduler::add_process(process *target_process)
    {
        if (!target_process) [[unlikely]]
        {
            // a9n::kernel::utility::logger::printk("invalid process\n");
            return scheduler_error::INVALID_PROCESS;
        }

        if (target_process->status != process_status::READY) [[unlikely]]
        {
            // in benno scheduling, only executable processes exist in the ready-queue
            // a9n::kernel::utility::logger::printk("invalid process status\n");
            return scheduler_error::INVALID_PROCESS;
        }

        auto target_priority = target_process->priority;
        if (target_priority < 0 || target_priority >= PRIORITY_MAX) [[unlikely]]
        {
            // a9n::kernel::utility::logger::printk("invalid priority\n");
            return scheduler_error::INVALID_PRIORITY;
        }

        if (target_process->next || target_process->preview) [[unlikely]]
        {
            DEBUG_LOG("process already exists in queue");
            // do nothing
            return {};
        }

        if (target_priority > highest_priority)
        {
            highest_priority = target_priority;
        }

        if (!queue[target_priority].head || !queue[target_priority].tail)
        {
            queue[target_priority].head = target_process;
            queue[target_priority].tail = target_process;

            return {};
        }

        queue[target_priority].tail->next = target_process;
        target_process->preview           = queue[target_priority].tail;
        queue[target_priority].tail       = target_process;
        target_process->next              = nullptr;

        return {};
    }
}
