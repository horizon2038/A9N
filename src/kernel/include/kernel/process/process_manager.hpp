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
    namespace
    {
        inline constexpr uint16_t PROCESS_COUNT_MAX = 128;
    }

    class process_manager
    {
      public:
        kernel_result init(void);

        kernel_result handle_timer(void);

        kernel_result switch_context(void);

        kernel_result switch_to_user(void);
        kernel_result switch_to_idle(void);

        kernel_result try_schedule_and_switch(void);
        kernel_result try_direct_schedule_and_switch(process &target_process);

        kernel_result yield(void);

        liba9n::result<process *, kernel_error> retrieve_current_process();
        kernel_result                           mark_scheduled(process &target_process);

      private:
        process   *current_process;
        a9n::sword highest_priority;

        alignas(a9n::WORD_BITS) scheduler scheduler_core {};
    };

    alignas(a9n::WORD_BITS) inline process_manager process_manager_core {};
}

#endif
