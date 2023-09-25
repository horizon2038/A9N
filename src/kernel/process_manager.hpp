#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <process.hpp>
#include "context_switch.hpp"

namespace kernel
{
    class process_manager
    {
        public:
            process_manager();
            ~process_manager();

            void create_process(const char *process_name, uint64_t process_address);
            void delete_process(int32_t process_id);

            static void schedule(); // called by timer-interrupt handler
    };
}

#endif
