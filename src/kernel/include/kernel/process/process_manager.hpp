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
        process   *current_process;
        a9n::sword highest_priority;

        kernel_result init(void);

        kernel_result switch_context(void);

        kernel_result switch_to_user(void);

        void create_process(const char *process_name, a9n::virtual_address entry_point_address);

        void init_process(
            process             *process,
            process_id           target_process_id,
            const char          *process_name,
            a9n::virtual_address entry_point_address
        );
        void delete_process(process_id target_process_id);

        process *search_process_from_id(process_id target_process_id);

        liba9n::result<process *, kernel_error> retrieve_current_process();

        kernel_result mark_scheduled(process &target_process);

      private:
        process    process_list[12];
        scheduler  scheduler_core {};
        a9n::sword determine_process_id();
    };

    inline process_manager process_manager_core {};
}

#endif
