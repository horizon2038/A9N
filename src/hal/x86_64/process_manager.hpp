#ifndef X86_64_CONTEXT_SWITCH_HPP
#define X86_64_CONTEXT_SWITCH_HPP

#include <interface/process_manager.hpp>

namespace hal::x86_64
{
    class process_manager final : hal::interface::process_manager
    {
        public:
            process_manager();
            ~process_manager();
            void switch_context(kernel::process *preview_process, kernel::process *next_process) override;
            void init_process(kernel::process *target_process, uint64_t pc) override;
            void delete_process(kernel::process *target_process) override;
        
        private:
    };
}

#endif
