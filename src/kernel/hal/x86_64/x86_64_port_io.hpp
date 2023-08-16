#ifndef X86_64_PORT_IO_HPP
#define X86_64_PORT_IO_HPP

#include "../include/port_io.hpp"

namespace hal::x86_64
{

    class port_io final : public hal::port_io
    {
        public:
            uint8_t read(uint16_t address) override;
            void write(uint16_t address, uint8_t data) override;
    };
}

#endif
