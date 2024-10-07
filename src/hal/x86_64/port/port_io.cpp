#include <hal/x86_64/io/port_io.hpp>

namespace a9n::hal::x86_64
{

    uint8_t port_io::read(uint16_t address)
    {
        return _port_read_8(address);
    }

    void port_io::write(uint16_t address, uint8_t data)
    {
        _port_write_8(address, data);
    }

    uint8_t port_read_8(uint16_t address)
    {
        return _port_read_8(address);
    }

    void port_write_8(uint16_t address, uint8_t data)
    {
        _port_write_8(address, data);
    }

    uint32_t port_read_32(uint16_t address)
    {
        return _port_read_32(address);
    }

    void port_write_32(uint16_t address, uint32_t data)
    {
        _port_write_32(address, data);
    }
}
