#ifndef PIC_HPP
#define PIC_HPP

// #include <interface/port_io.hpp>
#include <hal/x86_64/io/port_io.hpp>
#include <stdint.h>

namespace a9n::hal::x86_64
{
    enum class PIC_VALUE : uint16_t
    {
        PIC_MASTER         = 0x20,
        PIC_MASTER_COMMAND = 0x20,
        PIC_MASTER_DATA    = 0x21,
        PIC_SLAVE          = 0xa0,
        PIC_SLAVE_COMMAND  = 0xa0,
        PIC_SLAVE_DATA     = 0xa1
    };

    class pic
    {
      public:
        pic();
        ~pic();
        void init_pic();
        void remap_pic(uint8_t master_offset, uint8_t slave_offset);
        void wait();
        void mask(bool pic_flag, uint8_t mask_flag);
        void mask_irq(uint8_t irq_number);
        void unmask_irq(uint8_t irq_number);
        void end_of_interrupt_pic(uint8_t irq_number);

      private:
        a9n::hal::x86_64::port_io _port_io;
    };

    void disable_pic();
}

#endif
