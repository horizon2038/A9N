#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <process.hpp>
#include <interface/process_manager.hpp>
#include <interface/interrupt.hpp>
#include "scheduler.hpp"

namespace kernel
{
    namespace
    {
        inline constexpr uint16_t PROCESS_COUNT_MAX = 512;
    }

    class process_manager
    {
        public:
            process_manager(hal::interface::process_manager &target_process_manager);

            ~process_manager();

            process *current_process;

            void create_process(const char *process_name, virtual_address entry_point_address);
            void init_process(process *process, int32_t id, const char *process_name, virtual_address entry_point_address);
            void delete_process(int32_t process_id);
            void switch_context();

            process *search_process_from_id(int32_t process_id);

        private:
            process process_list[PROCESS_COUNT_MAX];

            scheduler _scheduler;
            hal::interface::process_manager &_process_manager;

            int32_t determine_process_id();
    };
}

#endif
