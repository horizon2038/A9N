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

    inline const char *scheduler_error_to_string(scheduler_error error)
    {
        switch (error)
        {
            case scheduler_error::NO_PROCESS :
                return "no process";
            case scheduler_error::INVALID_PROCESS :
                return "invalid process";
            case scheduler_error::INVALID_PRIORITY :
                return "invalid priority";
            case scheduler_error::CPU_ERROR :
                return "CPU error";
            case scheduler_error::INVALID_CPU_CORE :
                return "invalid CPU core";
            case scheduler_error::PROCESS_NOT_IN_QUEUE :
                return "process not in queue";
            case scheduler_error::PROCESS_ALREADY_EXISTS_IN_QUEUE :
                return "process already exists in queue";
            case scheduler_error::CAN_NOT_SCHEDULE :
                return "can not schedule";
            default :
                return "unknown error";
        }
    }

    struct process_queue
    {
        process *head { nullptr };
        process *tail { nullptr };
    };

    class scheduler
    {
      public:
        liba9n::result<process *, scheduler_error> schedule(void);
        liba9n::result<process *, scheduler_error> try_direct_schedule(process *target_process);
        scheduler_result                           add_process(process *target_process);

      private:
        process_queue queue[PRIORITY_MAX];
        a9n::sword    highest_priority { PRIORITY_MAX - 1 };
    };
}

#endif
