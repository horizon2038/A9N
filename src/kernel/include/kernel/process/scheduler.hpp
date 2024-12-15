#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "hal/interface/process_manager.hpp"
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <liba9n/result/result.hpp>
#include <stdint.h>

namespace a9n::kernel
{
    struct process_node
    {
        process      *process;
        process_node *next;
    };

    static constexpr a9n::sword PRIORITY_MAX = 32;

    enum class scheduler_error
    {
        NO_PROCESS,
        INVALID_PROCESS,
        INVALID_PRIORITY,
        CPU_ERROR,
        INVALID_CPU_CORE,
        PROCESS_NOT_IN_QUEUE,
        PROCESS_ALREADY_EXISTS_IN_QUEUE,
        CAN_NOT_SCHEDULE,
    };

    using scheduler_result = liba9n::result<void, scheduler_error>;

    struct process_queue
    {
        process *head { nullptr };
        process *tail { nullptr };
    };

    class scheduler
    {
      public:
        liba9n::result<process *, scheduler_error> schedule();
        liba9n::result<process *, scheduler_error> try_direct_schedule(process *target_process);
        scheduler_result                           add_process(process *target_process);

      private:
        process_queue queue[PRIORITY_MAX];
        a9n::sword    highest_priority { PRIORITY_MAX };
    };
}

#endif
