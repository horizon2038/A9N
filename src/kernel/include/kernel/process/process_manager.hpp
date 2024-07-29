#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <hal/interface/interrupt.hpp>
#include <hal/interface/process_manager.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/scheduler.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    namespace
    {
        inline constexpr uint16_t PROCESS_COUNT_MAX = 512;
    }

    class process_manager
    {
      public:
        process_manager(a9n::hal::process_manager &target_process_manager);

        ~process_manager();

        process   *current_process;
        a9n::sword highest_priority;

        void create_process(
            const char          *process_name,
            a9n::virtual_address entry_point_address
        );
        void init_process(
            process             *process,
            process_id           target_process_id,
            const char          *process_name,
            a9n::virtual_address entry_point_address
        );
        void delete_process(process_id target_process_id);
        void switch_context();

        process *search_process_from_id(process_id target_process_id);

      private:
        process  process_list[PROCESS_COUNT_MAX];
        process *priority_groups[PRIORITY_MAX] = { nullptr };

        scheduler                  _scheduler;
        a9n::hal::process_manager &_process_manager;

        a9n::sword determine_process_id();
    };
}

#endif
