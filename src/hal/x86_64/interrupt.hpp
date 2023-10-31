#ifndef X86_64_INTERRUPT_HPP
#define X86_64_INTERRUPT_HPP

#include <interface/interrupt.hpp>
#include "interrupt_descriptor.hpp"

#include "pic.hpp"

namespace hal::x86_64
{
    static inline hal::interface::interrupt_handler interrupt_handler_table[256];

    class interrupt final : public hal::interface::interrupt
    {
        public:
            interrupt();
            ~interrupt();
            void init_interrupt() override;
            void register_handler(uint32_t irq_number, hal::interface::interrupt_handler target_interrupt_handler) override;
            void enable_interrupt(uint32_t irq_number) override;
            void disable_interrupt(uint32_t irq_number) override;
            void enable_interrupt_all() override;
            void disable_interrupt_all() override;
            void mask_interrupt(hal::interface::interrupt_mask mask) override;
            void ack_interrupt() override;

        private:
            void init_handler();
            void load_idt();
            void register_idt_handler(uint32_t irq_number, hal::interface::interrupt_handler target_interrupt_handler);
            interrupt_descriptor_64 idt[256];
            pic _pic;

    };

}
#endif
