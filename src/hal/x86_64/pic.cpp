#include "pic.hpp"

namespace hal::x86_64
{
    namespace
    {
        constexpr uint16_t PIC_MASTER = 0x20;
        constexpr uint16_t PIC_MASTER_COMMAND = 0x20;
        constexpr uint16_t PIC_MASTER_DATA = 0x21;
        constexpr uint16_t PIC_SLAVE = 0xa0;
        constexpr uint16_t PIC_SLAVE_COMMAND = 0xa0;
        constexpr uint16_t PIC_SLAVE_DATA = 0xa1;
    }

    pic::pic(hal::interface::port_io &injected_port_io) : _port_io(injected_port_io)
    {
    }

    pic::~pic()
    {
    }

    void pic::init_pic()
    {
        _port_io.write(PIC_MASTER_COMMAND, 0x11); // send init command to master
        _port_io.write(PIC_SLAVE_COMMAND, 0x11); // send init command to slave

        _port_io.write(PIC_MASTER_DATA, 0x20); // assign master irq
        _port_io.write(PIC_SLAVE_DATA, 0x28); // assign slave irq

        _port_io.write(PIC_MASTER_DATA, 0x04); // notify connection to master
        _port_io.write(PIC_SLAVE_DATA, 0x02); // notify connection to slave

        _port_io.write(PIC_MASTER_DATA, 0x01); // set master to normal mode
        _port_io.write(PIC_SLAVE_DATA, 0x01); // set slave to normal mode

        // _port_io.write(PIC_MASTER_DATA, 0x00);
        // _port_io.write(PIC_SLAVE_DATA, 0x00);
    }

    void pic::end_of_interrupt_pic()
    {
        _port_io.write(PIC_MASTER_COMMAND, 0x20);
    }
}

