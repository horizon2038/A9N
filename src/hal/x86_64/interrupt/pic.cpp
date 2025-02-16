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

    inline constexpr uint8_t IMCR               = 0x22;
    inline constexpr uint8_t IMCR_DATA          = 0x23;

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

    inline static uint16_t pic_irq_mask = 0xFFFF & ~(1 << 2);

    namespace
    {
        union icw
        {
            struct icw1
            {
                enum command : uint8_t
                {
                    ICW4            = 1 << 0,
                    SINGLE          = 1 << 1,
                    INTERVAL_4      = 1 << 2,
                    LEVEL_TRIGGERED = 1 << 3,
                    INIT            = 1 << 4, // required
                };
            };

            struct icw4
            {
                enum command : uint8_t
                {
                    MODE_8086 = 1 << 0,
                    AUTO_EOI  = 1 << 1,
                    BUFFER    = 1 << 3,
                    NESTED    = 1 << 4,
                };
            };
        };

        union ocw
        {
            struct ocw2
            {
                enum command : uint8_t
                {
                    PRIORITY  = 1 << 0, // 3-bit; if SELECTION is set, change current priority
                    RESERVED  = 1 << 3, // 2-bit
                    EOI       = 1 << 5,
                    SELECTION = 1 << 6,
                    ROTATION  = 1 << 7,
                };
            };
        };
    }

    // test implementation (currently not working)
    void init_pic(void)
    {
#ifdef CONFIG_PIC
        // remap IRQs to 0x20+
        a9n::kernel::utility::logger::printh("PIC : remap all IRQs to 0x20+ ...\n");
        uint8_t master_offset = 0x20;
        uint8_t slave_offset  = 0x28;

        // init master
        // ICW1
        port_write_8(PIC_MASTER_COMMAND, (icw::icw1::INIT | icw::icw1::ICW4));
        port_wait();
        // ICW2
        port_write_8(PIC_MASTER_DATA, master_offset);
        port_wait();
        // ICW3
        port_write_8(PIC_MASTER_DATA, 0x04);
        port_wait();
        // ICW4
        port_write_8(PIC_MASTER_DATA, (icw::icw4::MODE_8086));
        port_wait();

        // init slave
        // ICW1
        port_write_8(PIC_SLAVE_COMMAND, (icw::icw1::INIT | icw::icw1::ICW4));
        port_wait();
        // ICW2
        port_write_8(PIC_SLAVE_DATA, slave_offset);
        port_wait();
        // ICW3
        port_write_8(PIC_SLAVE_DATA, 0x02);
        port_wait();
        // ICW4
        port_write_8(PIC_SLAVE_DATA, (icw::icw4::MODE_8086));
        port_wait();

        // mask
        // OCW1
        port_write_8(PIC_MASTER_DATA, 0xFF); // 2 is slave connection
        port_wait();
        port_write_8(PIC_SLAVE_DATA, 0xFF);
        port_wait();
#else
        port_write_8(PIC_MASTER_DATA, 0xFF);
        port_write_8(PIC_SLAVE_DATA, 0xFF);
#endif
    }

    void enable_pic(uint8_t irq_number)
    {
        mask_pic(pic_irq_mask & ~(1 << irq_number));
    }

    void mask_pic(uint16_t mask)
    {
        pic_irq_mask = mask;
        port_write_8(PIC_MASTER_DATA, mask);
        port_write_8(PIC_SLAVE_DATA, mask >> 8);
    }

    void end_of_interrupt_pic(uint8_t irq_number)
    {
        if (irq_number >= 8)
        {
            port_write_8(PIC_SLAVE_COMMAND, PIC_EOI);
        }
        port_write_8(PIC_MASTER_COMMAND, PIC_EOI);
    }

    void configure_imcr_to_apic(void)
    {
        port_write_8(IMCR, 0x70); // select IMCR
        port_wait();
        port_write_8(IMCR_DATA, 0x01); // PIC -> APIC
        port_wait();
    }

}
