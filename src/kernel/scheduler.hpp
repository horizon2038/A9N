#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <stdint.h>
#include <interface/arch_context_switch.hpp>

namespace kernel
{
    extern void _switch_context(uint64_t *current_stack_pointer, uint64_t *next_stack_pointer);

    class scheduler
    {
        public:
            scheduler();
            ~scheduler();

            __attribute__((interrupt("IRQ"))) static void clock(void *data);
            // TODO: remove attribute and update interrupt-system

        private:
            uint32_t tick;

    };
}

#endif
