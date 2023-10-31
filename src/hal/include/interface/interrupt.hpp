#ifndef HAL_INTERRUPT_HPP
#define HAL_INTERRUPT_HPP

#include <stdint.h>

namespace hal::interface
{

    using interrupt_handler = void (*)();

    typedef union
    {
        uint64_t mask_uint64[4];
        bool mask_bool[256];
    } interrupt_mask;

    enum class interrupt_type
    {
        INTERRUPT,
        EXCEPTION
    };

    class interrupt
    {
        public:
            virtual void init_interrupt() = 0;
            virtual void register_handler(uint32_t irq_number, interrupt_handler target_interrupt_handler) = 0;
            virtual void enable_interrupt(uint32_t irq_number) = 0;
            virtual void disable_interrupt(uint32_t irq_number) = 0;
            virtual void enable_interrupt_all() = 0;
            virtual void disable_interrupt_all() = 0;
            virtual void mask_interrupt(interrupt_mask mask) = 0;
            virtual void ack_interrupt() = 0;
    };

}
#endif
