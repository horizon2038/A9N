#ifndef X86_64_PORT_IO_HPP
#define X86_64_PORT_IO_HPP

#include <hal/interface/port_io.hpp>

namespace a9n::hal::x86_64
{
    extern "C" uint8_t _port_read_8(uint16_t address);
    extern "C" void    _port_write_8(uint16_t address, uint8_t data);

    extern "C" uint32_t _port_read_32(uint16_t address);
    extern "C" void     _port_write_32(uint16_t address, uint8_t data);

    uint8_t port_read_8(uint16_t address);
    void    port_write_8(uint16_t address, uint8_t data);

    uint32_t port_read_32(uint16_t address);
    void     port_write_32(uint16_t address, uint32_t data);

    class port_io final : public a9n::hal::port_io
    {
      public:
        uint8_t read(uint16_t address) override;
        void    write(uint16_t address, uint8_t data) override;

        inline static void io_wait()
        {
            do
            {
            } while (0);
        }
    };
}

#endif
