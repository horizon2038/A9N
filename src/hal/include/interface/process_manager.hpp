#ifndef HAL_CONTEXT_SWITCH_HPP
#define HAL_CONTEXT_SWITCH_HPP

#include <process/process.hpp>

namespace hal::interface
{
    class process_manager
    {
        public:
            virtual void switch_context(kernel::process *preview_process, kernel::process *next_process) = 0;
            virtual void create_process(kernel::process *target_process, kernel::virtual_address entry_point_address) = 0;
            virtual void delete_process(kernel::process *target_process) = 0;
    };
}

#endif
