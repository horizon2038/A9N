#include <hal/interface/port_io.hpp>

extern "C" uint8_t  _port_read_8(uint16_t address);
extern "C" void     _port_write_8(uint16_t address, uint8_t data);
extern "C" uint32_t _port_read_32(uint16_t address);
extern "C" void     _port_write_32(uint16_t address, uint32_t data);

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> read_io_port(a9n::word address)
    {
        if (!address)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        return static_cast<a9n::word>(_port_read_8(static_cast<uint16_t>(address)));
    }

    hal_result write_io_port(a9n::word address, a9n::word data)
    {
        _port_write_8(static_cast<uint16_t>(address), static_cast<uint8_t>(data));
        return {};
    }
}
