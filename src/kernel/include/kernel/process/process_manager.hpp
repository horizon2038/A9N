#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <hal/interface/interrupt.hpp>
#include <hal/interface/process_manager.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/scheduler.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    struct cpu_local_variable;

    class process_manager
    {
      public:
        kernel_result init(a9n::word core_number);

        kernel_result handle_timer(void);

        kernel_result switch_to_user(void);
        kernel_result switch_to_idle(void);

        kernel_result try_schedule_and_switch(void);
        kernel_result try_schedule_and_switch(cpu_local_variable &local_variable);
        kernel_result try_direct_schedule_and_switch(process &target_process);
        kernel_result
            try_direct_schedule_and_switch(process &target_process, cpu_local_variable &local_variable);

        kernel_result schedule_if_preempted_by(process &target);

        kernel_result yield(void);

        liba9n::result<process *, kernel_error> retrieve_current_process();
        kernel_result                           mark_scheduled(process &target_process);
        kernel_result                           mark_suspended(process &target_process);

      private:
        process   *current_process;
        a9n::sword highest_priority;

        process idle_process {};
        alignas(a9n::WORD_BITS) scheduler scheduler_core {};
    };

    kernel_result init_idle_context(void);

    liba9n::result<process *, kernel_error>         current_process_on_this_core(void);
    liba9n::result<process_manager *, kernel_error> current_process_manager(void);
    kernel_result                                   reschedule_core(a9n::word core_number);
    kernel_result                                   try_schedule_and_switch(process &current);
    kernel_result try_direct_schedule_and_switch(process &current, process &target);
    kernel_result mark_scheduled(process &current, process &target);
    kernel_result mark_scheduled_with_preemption(process &current, process &target);
}

#endif
