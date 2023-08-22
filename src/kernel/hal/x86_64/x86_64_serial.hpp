#ifndef X86_64_SERIAL_HPP
#define X86_64_SERIAL_HPP

#include <include/serial.hpp>

#include <stdint.h>
#include <include/port_io.hpp>

namespace hal::x86_64
{
    class serial : public hal::serial
    {
        public:
            serial(hal::port_io &injected_port_io);
            ~serial();
            void init_serial(uint32_t baud_rate) override;
            uint8_t read_serial() override;
            void write_serial(char data) override;
            void write_string_serial(char *out) override;

        private:
            hal::port_io &_port_io;
            int is_received();
            int is_empty();
            uint32_t strlen(const char *s);

    };

}

#endif
