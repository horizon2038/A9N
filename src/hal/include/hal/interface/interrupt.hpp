#ifndef HAL_INTERRUPT_HPP
#define HAL_INTERRUPT_HPP

#include <library/common/types.hpp>
#include <stdint.h>

namespace hal::interface
{

    using interrupt_handler = void (*)();

    enum class interrupt_type
    {
        INTERRUPT,
        EXCEPTION
    };

    class interrupt
    {
      public:
        virtual void init_interrupt() = 0;
        virtual void register_handler(
            common::word      irq_number,
            interrupt_handler target_interrupt_handler
        )                                                       = 0;
        virtual void enable_interrupt(common::word irq_number)  = 0;
        virtual void disable_interrupt(common::word irq_number) = 0;
        virtual void enable_interrupt_all()                     = 0;
        virtual void disable_interrupt_all()                    = 0;
        virtual void ack_interrupt()                            = 0;
    };

}
#endif
