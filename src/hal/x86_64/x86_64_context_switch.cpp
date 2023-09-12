#include "x86_64_context_switch.hpp"

extern void _switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer);

namespace hal::x86_64
{
    context_switch::context_switch()
    {
    }

    context_switch::~context_switch()
    {
    }

    void context_switch::switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer)
    {
    }
}
