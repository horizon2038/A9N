#ifndef X86_64_SERIAL_HPP
#define X86_64_SERIAL_HPP

#include <hal/interface/serial.hpp>

#include <hal/interface/port_io.hpp>
#include <stdint.h>

namespace hal::x86_64
{
    class serial : public hal::interface::serial
    {
      public:
        serial(hal::interface::port_io &injected_port_io);
        ~serial();
        void    init_serial(common::word baud_rate) override;
        uint8_t read_serial() override;
        void    write_serial(char data) override;
        void    write_string_serial(char *out) override;

      private:
        hal::interface::port_io &_port_io;
        int                      is_received();
        int                      is_empty();
        uint32_t                 strlen(const char *s);
    };

}

#endif
