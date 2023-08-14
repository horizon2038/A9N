#ifndef X86_64_INTERRUPT_HPP
#define X86_64_INTERRUPT_HPP

#include "../include/interrupt.hpp"

class x86_64_interrupt : public interrupt
{
    public:
        void init_interrupt() override;
        void register_interrupt(uint32_t irq_number, void *target_irq_handler) override;
        void enable_interrupt(uint32_t irq_number) override;
        void disable_interrupt(uint32_t irq_number) override;
        void enable_interrupt_all() override;
        void disable_interrupt_all() override;
        void mask_interrupt(uint64_t mask) override;
};

#endif
