#ifndef PIC_HPP
#define PIC_HPP

#include <interface/port_io.hpp>

namespace hal::x86_64
{
    enum class PIC_VALUE : uint16_t
    {
        PIC_MASTER = 0x20,
        PIC_MASTER_COMMAND = 0x20,
        PIC_MASTER_DATA = 0x21,
        PIC_SLAVE = 0xa0,
        PIC_SLAVE_COMMAND = 0xa0,
        PIC_SLAVE_DATA = 0xa1
    };

    class pic
    {
        public:
            pic(hal::interface::port_io &injected_port_io);
            ~pic();
            void init_pic();
            void end_of_interrupt_pic();

        private:
            hal::interface::port_io &_port_io;
    };
}

#endif
