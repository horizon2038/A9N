#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <library/common/types.hpp>
#include <kernel/process/process.hpp>
#include <hal/interface/process_manager.hpp>
#include <hal/interface/interrupt.hpp>
#include <kernel/process/scheduler.hpp>

namespace kernel
{
    namespace
    {
        inline constexpr uint16_t PROCESS_COUNT_MAX = 512;
    }

    class process_manager
    {
      public:
        process_manager(hal::interface::process_manager &target_process_manager
        );

        ~process_manager();

        process      *current_process;
        common::sword highest_priority;

        void create_process(
            const char             *process_name,
            common::virtual_address entry_point_address
        );
        void init_process(
            process                *process,
            process_id              target_process_id,
            const char             *process_name,
            common::virtual_address entry_point_address
        );
        void delete_process(process_id target_process_id);
        void switch_context();

        process *search_process_from_id(process_id target_process_id);

      private:
        process  process_list[PROCESS_COUNT_MAX];
        process *priority_groups[PRIORITY_MAX] = { nullptr };

        scheduler                        _scheduler;
        hal::interface::process_manager &_process_manager;

        common::sword determine_process_id();
    };
}

#endif
