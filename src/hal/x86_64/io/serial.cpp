#include <hal/x86_64/io/serial.hpp>

#include <hal/interface/interrupt.hpp>
#include <hal/interface/port_io.hpp>
#include <hal/x86_64/interrupt/pic.hpp>
#include <kernel/types.hpp>

// for debug
#include <hal/x86_64/interrupt/apic.hpp>
#include <hal/x86_64/io/port_io.hpp>

namespace a9n::hal::x86_64
{
    inline constexpr uint16_t COM_1 = 0x3f8;
    inline constexpr uint16_t COM_2 = 0x2f8;
    inline constexpr uint16_t COM_3 = 0x3e8;
    inline constexpr uint16_t COM_4 = 0x2e8;

    inline constexpr uint16_t RBR   = 0;
    inline constexpr uint16_t DLL   = 0;
    inline constexpr uint16_t DLH   = 1;
    inline constexpr uint16_t IER   = 1;
    inline constexpr uint16_t FCR   = 2;
    inline constexpr uint16_t LCR   = 3;
    inline constexpr uint16_t MCR   = 4;
    inline constexpr uint16_t LSR   = 5;

    serial::serial(a9n::hal::port_io &injected_port_io) : _port_io(injected_port_io)
    {
    }

    serial::~serial()
    {
    }

    void serial::init_serial(a9n::word baud_rate)
    {
        // TODO: return hal_error::ILLEGAL_ARGUMENT
        if (baud_rate == 0)
        {
            return;
        }

        // // disable fifo
        // port_write_8(COM_1 + FCR, 0x00);
        // enable fifo
        port_write_8(COM_1 + FCR, 0xc7);
        port_wait();
        // disable interrupts
        port_write_8(COM_1 + IER, 0x00);
        port_wait();

        // configure 8bits, no parity, one stop bit
        port_write_8(COM_1 + LCR, 0x80); // 0ld: 0x03
        port_wait();

        // lock divisor
        port_write_8(COM_1 + LCR, 1 << 7);
        port_wait();

        // configure divisor
        int divisor = 115200 / baud_rate;
        port_write_8(COM_1 + DLL, divisor & 0xff);
        port_wait();
        port_write_8(COM_1 + DLH, (divisor >> 8) & 0xff);
        port_wait();

        // disable dlab
        port_write_8(COM_1 + LCR, 0x03);
        port_wait();
    }

    uint8_t serial::read_serial()
    {
        while (is_received() == 0)
            ;
        return this->_port_io.read(COM_1);
    }

    int serial::is_received()
    {
        return this->_port_io.read(COM_1 + 5) & 0x01;
    }

    void serial::write_serial(char data)
    {
        while (is_empty() == 0)
            ;
        this->_port_io.write(COM_1, data);
    }

    int serial::is_empty()
    {
        return this->_port_io.read(COM_1 + 5) & 0x20;
    }

    void serial::write_string_serial(char *words)
    {
        for (uint32_t i = 0; i < strlen(words); ++i)
        {
            write_serial(words[i]);
        }
    }

    uint32_t serial::strlen(const char *s)
    {
        const char *ss;
        for (ss = s; *ss != '\0'; ss++)
            ;
        return ss - s;
    }

    int is_received(void)
    {
        return port_read_8(COM_1 + 5) & 0x01;
    }

    uint8_t read_serial(void)
    {
        while (is_received() == 0)
            ;
        return port_read_8(COM_1);
    }

    void reconfigure_serial(void)
    {
        // UART re-configuration

        // !!! enable OUT2 in MCR !!!
        port_write_8(COM_1 + MCR, 0x0B);

        // enable receive interrupt
        port_write_8(COM_1 + IER, 0x01);

        // ack interrupt
        port_read_8(COM_1 + FCR);
        port_read_8(COM_1);

        io_apic_core.enable_interrupt(0x04);
        local_apic_core.end_of_interrupt();
    }
};
