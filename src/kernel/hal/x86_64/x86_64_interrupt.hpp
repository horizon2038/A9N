#ifndef X86_64_INTERRUPT_HPP
#define X86_64_INTERRUPT_HPP

#include "../include/interrupt.hpp"
#include "interrupt_descriptor.hpp"
#include "../include/interrupt_handler.hpp"

namespace hal::x86_64
{
    class interrupt final : public hal::interrupt
    {
        public:
            interrupt();
            ~interrupt();
            void init_interrupt() override;
            void register_interrupt(uint32_t irq_number, interrupt_handler &target_interrupt_handler) override;
            void enable_interrupt(uint32_t irq_number) override;
            void disable_interrupt(uint32_t irq_number) override;
            void enable_interrupt_all() override;
            void disable_interrupt_all() override;
            void mask_interrupt(hal::interrupt_mask mask) override;

        private:
            void load_idt();
            interrupt_descriptor_64 idt[256];

    };

}
#endif
