#include <hal/interface/port_io.hpp>
#include <hal/x86_64/io/port_io.hpp>

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> read_io_port(a9n::word address, a9n::word byte_width)
    {
        switch (byte_width)
        {
            case 1 :
                return static_cast<a9n::word>(
                    x86_64::port_read_8(static_cast<uint16_t>(address)) & 0xFF
                );
            case 2 :
                return static_cast<a9n::word>(
                    x86_64::port_read_16(static_cast<uint16_t>(address)) & 0xFFFF
                );
            case 4 :
                return static_cast<a9n::word>(
                    x86_64::port_read_32(static_cast<uint16_t>(address)) & 0xFFFF'FFFF
                );
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }

    hal_result write_io_port(a9n::word address, a9n::word byte_width, a9n::word data)
    {
        switch (byte_width)
        {
            case 1 :
                x86_64::port_write_8(static_cast<uint16_t>(address), static_cast<uint8_t>(data & 0xFF));
                return {};
            case 2 :
                x86_64::port_write_16(
                    static_cast<uint16_t>(address),
                    static_cast<uint16_t>(data & 0xFFFF)
                );
                return {};
            case 4 :
                x86_64::port_write_32(
                    static_cast<uint16_t>(address),
                    static_cast<uint32_t>(data & 0xFFFF'FFFF)
                );
                return {};
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }
}
