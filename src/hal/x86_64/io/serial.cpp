#include <hal/interface/serial.hpp>
#include <hal/x86_64/io/port_io.hpp>

namespace a9n::hal::x86_64
{
    inline constexpr uint16_t COM_1 = 0x3f8;
    inline constexpr uint16_t RBR   = 0;
    inline constexpr uint16_t DLL   = 0;
    inline constexpr uint16_t DLH   = 1;
    inline constexpr uint16_t IER   = 1;
    inline constexpr uint16_t FCR   = 2;
    inline constexpr uint16_t LCR   = 3;
    inline constexpr uint16_t MCR   = 4;
    inline constexpr uint16_t LSR   = 5;

    void reconfigure_serial()
    {
        port_write_8(COM_1 + MCR, 0x0b);
        port_write_8(COM_1 + FCR, 0x07);
        port_write_8(COM_1 + IER, 0x01);
    }
}

namespace a9n::hal
{
    void init_serial(a9n::word baud_rate)
    {
        using namespace x86_64;
        if (!baud_rate)
        {
            baud_rate = 115200;
        }

        port_write_8(COM_1 + FCR, 0xc7);
        port_wait();
        port_write_8(COM_1 + IER, 0x00);
        port_wait();
        port_write_8(COM_1 + LCR, 1U << 7);
        port_wait();

        const auto divisor = static_cast<uint16_t>(115200 / baud_rate);
        port_write_8(COM_1 + DLL, static_cast<uint8_t>(divisor & 0xff));
        port_wait();
        port_write_8(COM_1 + DLH, static_cast<uint8_t>(divisor >> 8));
        port_wait();
        port_write_8(COM_1 + LCR, 0x03);
        port_wait();
    }

    uint8_t read_serial()
    {
        while ((x86_64::port_read_8(x86_64::COM_1 + x86_64::LSR) & 0x01) == 0)
        {
            asm volatile("pause");
        }
        return x86_64::port_read_8(x86_64::COM_1 + x86_64::RBR);
    }

    void write_serial(char data)
    {
        while ((x86_64::port_read_8(x86_64::COM_1 + x86_64::LSR) & 0x20) == 0)
        {
            asm volatile("pause");
        }
        x86_64::port_write_8(x86_64::COM_1, static_cast<uint8_t>(data));
    }

    void write_string_serial(const char *out)
    {
        if (!out)
        {
            return;
        }
        while (*out)
        {
            write_serial(*out++);
        }
    }
}
