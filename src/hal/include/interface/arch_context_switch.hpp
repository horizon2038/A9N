#ifndef CONTEXT_SWITCH_HPP
#define CONTEXT_SWITCH_HPP

#include <stdint.h>

namespace hal::interface
{
    class arch_context_switch
    {
        virtual void switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer) = 0;
    };
}

#endif
