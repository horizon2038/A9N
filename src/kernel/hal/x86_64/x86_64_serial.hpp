#ifndef X86_64_SERIAL_HPP
#define X86_64_SERIAL_HPP

#include <stdint.h>
#include <include/port_io.hpp>

namespace hal::x86_64
{
    class serial
    {
        public:
            serial(hal::port_io &injected_port_io);
            ~serial();
            void init_serial(uint32_t baud_rate);
            int read_serial();
            void write_serial(char data);
            void write_string_serial(char *out);

        private:
            hal::port_io &target_port_io;
            int is_received();
            int is_empty();
            uint32_t strlen(const char *s);

    };

}

#endif
