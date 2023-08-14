#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP

#include <stdint.h>

class interrupt
{
    public:
        virtual void init_interrupt() = 0;
        virtual void register_interrupt(uint32_t irq_number, void *target_irq_handler) = 0;
        virtual void enable_interrupt(uint32_t irq_number) = 0;
        virtual void disable_interrupt(uint32_t irq_number) = 0;
        virtual void enable_interrupt_all() = 0;
        virtual void disable_interrupt_all() = 0;
        virtual void mask_interrupt(uint64_t mask) = 0;
};

#endif
