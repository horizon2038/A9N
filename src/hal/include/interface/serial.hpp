#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <stdint.h>

namespace hal::interface
{
    class serial
    {
        public:
            virtual void init_serial(uint32_t baud_rate) = 0;
            virtual uint8_t read_serial() = 0;
            virtual void write_serial(char data) = 0;
            virtual void write_string_serial(char *out) = 0;
    };
}

#endif
