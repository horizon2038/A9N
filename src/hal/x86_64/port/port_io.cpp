#include "port_io.hpp"

namespace hal::x86_64
{
    extern "C" uint8_t _read(uint16_t address);
    extern "C" void _write(uint16_t address, uint8_t data);

    uint8_t port_io::read(uint16_t address)
    {
        return _read(address);
    }

    void port_io::write(uint16_t address, uint8_t data)
    {
        _write(address, data);
    }
}
