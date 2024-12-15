#ifndef HAL_PORT_IO_HPP
#define HAL_PORT_IO_HPP

#include <stdint.h>

#include <kernel/types.hpp>

namespace a9n::hal
{
    class port_io
    {
      public:
        virtual uint8_t read(uint16_t address)                = 0;
        virtual void    write(uint16_t address, uint8_t data) = 0;
    };

    uint8_t read_io_port(a9n::word address);
    uint8_t write_io_port(a9n::word address);
}

#endif
