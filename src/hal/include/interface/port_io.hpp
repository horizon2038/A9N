#ifndef PORT_IO_HPP
#define PORT_IO_HPP

#include <stdint.h>

namespace hal::interface
{
    class port_io
    {
        public:
            virtual uint8_t read(uint16_t address) = 0;
            virtual void write(uint16_t address, uint8_t data) = 0;
    };

}

#endif
