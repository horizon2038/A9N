#ifndef HAL_CONTEXT_SWITCH_HPP
#define HAL_CONTEXT_SWITCH_HPP

#include <stdint.h>
#include <process.hpp>

namespace hal::interface
{
    class process_manager
    {
        public:
            virtual void switch_context(kernel::process *preview_process, kernel::process *next_process) = 0;
            // TODO: change switch_context to process received.
            virtual void init_process(kernel::process *target_process) = 0;
            virtual void delete_process(kernel::process *target_process) = 0;
    };
}

#endif
