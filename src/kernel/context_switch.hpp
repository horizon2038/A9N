#ifndef CONTEXT_SWITCH_HPP
#define CONTEXT_SWITCH_HPP

#include <interface/process_manager.hpp>
#include <interface/interrupt.hpp>

#include <process.hpp>

namespace kernel
{
    class context_switch
    {
        public:
            context_switch
            (
                hal::interface::process_manager &injected_process_manager,
                hal::interface::interrupt &inject_interrupt
            );
            ~context_switch();
            void switch_context(process *preview_process, process *next_process);

        private:
            hal::interface::process_manager &_process_manager;
            hal::interface::interrupt &_interrupt;
    };
}

#endif
