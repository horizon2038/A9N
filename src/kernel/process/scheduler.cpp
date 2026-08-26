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
        if (!target_process) [[unlikely]]
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

            add_process(target_process);

            return schedule();
        }

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
            DEBUG_LOG("invalid process");
            return scheduler_error::INVALID_PROCESS;
        }

        if (target_process->status != process_status::READY) [[unlikely]]
        {
            // in benno scheduling, only executable processes exist in the ready-queue
            DEBUG_LOG("invalid process status");
            return scheduler_error::INVALID_PROCESS;
        }

        auto target_priority = target_process->priority;
        if (target_priority < 0 || target_priority >= PRIORITY_MAX) [[unlikely]]
        {
            DEBUG_LOG("invalid priority");
            return scheduler_error::INVALID_PRIORITY;
        }

        if (target_process->next || target_process->preview) [[unlikely]]
        {
            DEBUG_LOG("process already exists in queue");
            return scheduler_error::PROCESS_ALREADY_EXISTS_IN_QUEUE;
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

    scheduler_result scheduler::remove_process(process *target_process)
    {
        if (!target_process) [[unlikely]]
        {
            return scheduler_error::INVALID_PROCESS;
        }

        for (a9n::sword priority = 0; priority < PRIORITY_MAX; priority++)
        {
            process *previous = nullptr;
            process *current  = queue[priority].head;

            while (current)
            {
                if (current == target_process)
                {
                    auto *next = current->next;
                    if (previous)
                    {
                        previous->next = next;
                    }
                    else
                    {
                        queue[priority].head = next;
                    }

                    if (next)
                    {
                        next->preview = previous;
                    }
                    else
                    {
                        queue[priority].tail = previous;
                    }

                    current->next    = nullptr;
                    current->preview = nullptr;

                    while (highest_priority > 0 && !queue[highest_priority].head)
                    {
                        highest_priority--;
                    }
                    return {};
                }

                previous = current;
                current  = current->next;
            }
        }

        return scheduler_error::PROCESS_NOT_IN_QUEUE;
    }
}
