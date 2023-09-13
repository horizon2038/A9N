#include "serial.hpp"

#include "port_io.hpp"
#include <interface/port_io.hpp>
#include <cpp_dependent/new.hpp>

namespace hal::x86_64
{
    constexpr static uint16_t COM_1 = 0x3f8;
    constexpr static uint16_t COM_2 = 0x2f8;
    constexpr static uint16_t COM_3 = 0x3e8;
    constexpr static uint16_t COM_4 = 0x2e8;

    serial::serial(hal::interface::port_io &injected_port_io) : _port_io(injected_port_io)
    {
        
    }

    serial::~serial()
    {
    }

    void serial::init_serial(uint32_t baud_rate)
    {
        this->_port_io.write(COM_1 + 1, 0x00);    // Disable all interrupts
        this->_port_io.write(COM_1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
        this->_port_io.write(COM_1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
        this->_port_io.write(COM_1 + 1, 0x00);    //                  (hi byte)
        this->_port_io.write(COM_1 + 3, 0x03);    // 8 bits, no parity, one stop bit
        this->_port_io.write(COM_1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        this->_port_io.write(COM_1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
        // If serial is not faulty set it in normal operation mode
        // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    }

    uint8_t serial::read_serial()
    {
        return 0;
    }

    int serial::is_received()
    {
        return 0;
    }

    void serial::write_serial(char data)
    {
        while (is_empty() == 0);
        this->_port_io.write(COM_1,data);
    }

    int serial::is_empty()
    {
        return this->_port_io.read(COM_1 + 5) & 1;
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
      const char* ss;
      for (ss = s; *ss != '\0'; ss++);
      return ss - s;
    }

};
