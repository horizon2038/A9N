#include "x86_64_context_switch.hpp"

extern void _switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer);

namespace hal::x86_64
{
    arch_context_switch::arch_context_switch()
    {
    }

    arch_context_switch::~arch_context_switch()
    {
    }

    void arch_context_switch::switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer)
    {
    }
}
