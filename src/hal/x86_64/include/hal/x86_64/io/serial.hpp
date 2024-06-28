#ifndef X86_64_SERIAL_HPP
#define X86_64_SERIAL_HPP

#include <hal/interface/serial.hpp>

#include <hal/interface/port_io.hpp>
#include <stdint.h>

namespace a9n::hal::x86_64
{
    class serial : public a9n::hal::serial
    {
      public:
        serial(a9n::hal::port_io &injected_port_io);
        ~serial();
        void    init_serial(a9n::word baud_rate) override;
        uint8_t read_serial() override;
        void    write_serial(char data) override;
        void    write_string_serial(char *out) override;

      private:
        a9n::hal::port_io &_port_io;
        int                      is_received();
        int                      is_empty();
        uint32_t                 strlen(const char *s);
    };

}

#endif
