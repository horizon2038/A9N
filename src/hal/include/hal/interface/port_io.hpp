#ifndef HAL_PORT_IO_HPP
#define HAL_PORT_IO_HPP

#include <stdint.h>

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

namespace a9n::hal
{
    class port_io
    {
      public:
        virtual uint8_t read(uint16_t address)                = 0;
        virtual void    write(uint16_t address, uint8_t data) = 0;
    };

    liba9n::result<a9n::word, hal_error> read_io_port(a9n::word address);
    hal_result                           write_io_port(a9n::word address, a9n::word data);
}

#endif
