#include <hal/x86_64/interrupt/pic.hpp>

#include <hal/x86_64/io/port_io.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    inline constexpr uint8_t PIC_MASTER         = 0x20;
    inline constexpr uint8_t PIC_MASTER_COMMAND = 0x20;
    inline constexpr uint8_t PIC_MASTER_DATA    = 0x21;
    inline constexpr uint8_t PIC_SLAVE          = 0xa0;
    inline constexpr uint8_t PIC_SLAVE_COMMAND  = 0xa0;
    inline constexpr uint8_t PIC_SLAVE_DATA     = 0xa1;
    inline constexpr uint8_t PIC_EOI            = 0x20;

    pic::pic() : _port_io()
    {
    }

    pic::~pic()
    {
    }

    void pic::init_pic()
    {
        remap_pic(32, 40);
    }

    void pic::remap_pic(uint8_t master_offset, uint8_t slave_offset)
    {
        uint8_t master_mask;
        uint8_t slave_mask;

        master_mask = _port_io.read(PIC_MASTER_DATA);
        slave_mask  = _port_io.read(PIC_SLAVE_DATA);

        _port_io.write(PIC_MASTER_COMMAND, 0x11); // send init command to master
        _port_io.io_wait();
        _port_io.write(PIC_SLAVE_COMMAND, 0x11); // send init command to slave
        _port_io.io_wait();

        _port_io.write(PIC_MASTER_DATA, master_offset); // assign master irq
        _port_io.io_wait();
        _port_io.write(PIC_SLAVE_DATA, slave_offset); // assign slave irq
        _port_io.io_wait();

        _port_io.write(PIC_MASTER_DATA, 0x04); // notify connection to master
        _port_io.io_wait();
        _port_io.write(PIC_SLAVE_DATA, 0x02); // notify connection to slave
        _port_io.io_wait();

        _port_io.write(PIC_MASTER_DATA, 0x01); // set master to normal mode
        _port_io.io_wait();
        _port_io.write(PIC_SLAVE_DATA, 0x01); // set slave to normal mode
        _port_io.io_wait();

        _port_io.write(PIC_MASTER_DATA, 0xff);
        _port_io.io_wait();
        _port_io.write(PIC_SLAVE_DATA, 0xff);
        _port_io.io_wait();

        _port_io.write(PIC_MASTER_COMMAND, 0x20);
        _port_io.io_wait();

        // _port_io.write(PIC_MASTER_DATA, master_mask);
        // _port_io.write(PIC_SLAVE_DATA, slave_mask);
    }

    void pic::wait()
    {
        _port_io.write(0x80, 0);
    }

    void pic::mask(bool pic_flag, uint8_t mask_flag)
    {
        if (!pic_flag)
        {
            _port_io.write(PIC_MASTER_DATA, mask_flag);
        }
        else
        {
            _port_io.write(PIC_SLAVE_DATA, mask_flag);
        }
    }

    void pic::mask_irq(uint8_t irq_number)
    {
        uint16_t port;
        uint8_t  value;

        port = PIC_MASTER_DATA;

        if (irq_number > 8)
        {
            port        = PIC_SLAVE_DATA;
            irq_number -= 8;
        }

        value = _port_io.read(port) | (1 << irq_number);
        _port_io.write(port, value);
    }

    void pic::unmask_irq(uint8_t irq_number)
    {
        uint16_t port;
        uint8_t  value;

        port = PIC_MASTER_DATA;

        if (irq_number > 8)
        {
            port        = PIC_SLAVE_DATA;
            irq_number -= 8;
        }

        value = _port_io.read(port) & ~(1 << irq_number);
        _port_io.write(port, value);

        uint8_t master_mask = _port_io.read(PIC_MASTER_DATA);
        uint8_t slave_mask  = _port_io.read(PIC_SLAVE_DATA);
    }

    void pic::end_of_interrupt_pic(uint8_t irq_number)
    {
        if (irq_number >= 8)
        {
            _port_io.write(PIC_SLAVE_COMMAND, PIC_EOI);
        }
        _port_io.write(PIC_MASTER_COMMAND, PIC_EOI);
    }

    void disable_pic()
    {
        /*
        // mask master pic
        port_write_8(PIC_MASTER_DATA, 0xFF);
        // mask slave pic
        port_write_8(PIC_SLAVE_DATA, 0xFF);

        // icw1
        port_write_8(PIC_MASTER_COMMAND, 0x11); // send init command to master
        port_write_8(PIC_SLAVE_COMMAND, 0x11);  // send init command to slave

        // icw2
        port_write_8(PIC_MASTER_DATA, 0x20); // notify connection to master
        port_write_8(PIC_SLAVE_DATA, 0x028); // notify connection to slave

        // icw3
        port_write_8(PIC_MASTER_DATA, 0x04); // set master to normal mode
        port_write_8(PIC_SLAVE_DATA, 0x02);  // set slave to normal mode

        // icw4 : 8086
        port_write_8(PIC_MASTER_DATA, 0x01);
        port_write_8(PIC_SLAVE_DATA, 0x01);
        */

        // mask all interrupts
        port_write_8(PIC_MASTER_DATA, 0xFF);
        port_write_8(PIC_SLAVE_DATA, 0xFF);
    }

}
