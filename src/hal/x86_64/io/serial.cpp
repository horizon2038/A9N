#include <hal/x86_64/io/serial.hpp>

#include <hal/interface/port_io.hpp>
#include <hal/x86_64/io/port_io.hpp>

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    static constexpr uint16_t COM_1 = 0x3f8;
    static constexpr uint16_t COM_2 = 0x2f8;
    static constexpr uint16_t COM_3 = 0x3e8;
    static constexpr uint16_t COM_4 = 0x2e8;

    serial::serial(a9n::hal::port_io &injected_port_io)
        : _port_io(injected_port_io)
    {
    }

    serial::~serial()
    {
    }

    void serial::init_serial(a9n::word baud_rate)
    {
        this->_port_io.write(COM_1 + 1, 0x00);
        this->_port_io.write(COM_1 + 3, 0x80);
        this->_port_io.write(COM_1 + 0, 0x03);
        this->_port_io.write(COM_1 + 1, 0x00);
        this->_port_io.write(COM_1 + 3, 0x03);
        this->_port_io.write(COM_1 + 2, 0xc7);
        this->_port_io.write(COM_1 + 4, 0x0b);
        this->_port_io.write(COM_1 + 4, 0x0f);
        this->_port_io.write(COM_1 + 1, 0x03);
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

};
