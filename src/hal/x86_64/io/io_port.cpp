#include <hal/interface/port_io.hpp>

extern "C" uint8_t  _port_read_8(uint16_t address);
extern "C" void     _port_write_8(uint16_t address, uint8_t data);
extern "C" uint16_t _port_read_16(uint16_t address);
extern "C" void     _port_write_16(uint16_t address, uint16_t data);
extern "C" uint32_t _port_read_32(uint16_t address);
extern "C" void     _port_write_32(uint16_t address, uint32_t data);

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> read_io_port(a9n::word address, a9n::word byte_width)
    {
        switch (byte_width)
        {
            case 1 :
                return static_cast<a9n::word>(_port_read_8(static_cast<uint16_t>(address)) & 0xFF);
            case 2 :
                return static_cast<a9n::word>(_port_read_16(static_cast<uint16_t>(address)) & 0xFFFF);
            case 4 :
                return static_cast<a9n::word>(
                    _port_read_32(static_cast<uint16_t>(address)) & 0xFFFF'FFFF
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
                _port_write_8(static_cast<uint16_t>(address), static_cast<uint8_t>(data & 0xFF));
                return {};
            case 2 :
                _port_write_16(static_cast<uint16_t>(address), static_cast<uint16_t>(data & 0xFFFF));
                return {};
            case 4 :
                _port_write_32(static_cast<uint16_t>(address), static_cast<uint32_t>(data & 0xFFFF'FFFF));
                return {};
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
        _port_write_8(static_cast<uint16_t>(address), static_cast<uint8_t>(data));
        return {};
    }
}
