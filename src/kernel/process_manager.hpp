#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <process.hpp>
#include "context_switch.hpp"

namespace kernel
{
    class process_manager
    {
        public:
            process_manager
            (
                hal::interface::process_manager &injected_process_manager,
                hal::interface::interrupt &inject_interrupt
            );

            ~process_manager();

            void create_process(const char *process_name, uint64_t process_address);
            void delete_process(int32_t process_id);
            void switch_context(process *preview_process, process *next_process);

            static void schedule(); // called by timer-interrupt handler

        private:
            hal::interface::process_manager &_process_manager;
            hal::interface::interrupt &_interrupt;
    };
}

#endif
