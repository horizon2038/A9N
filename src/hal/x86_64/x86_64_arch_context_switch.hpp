#ifndef X86_64_CONTEXT_SWITCH_HPP
#define X86_64_CONTEXT_SWITCH_HPP

#include <interface/context_switch.hpp>

namespace hal::x86_64
{
    class arch_context_switch final : hal::interface::arch_context_switch
    {
        arch_context_switch();
        ~arch_context_switch();
        void switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer) override;
    };
}

#endif
