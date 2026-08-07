#include <hal/x86_64/io/port_io.hpp>

namespace a9n::hal::x86_64
{

    uint8_t port_io::read(uint16_t address)
    {
        return port_read_8(address);
    }

    void port_io::write(uint16_t address, uint8_t data)
    {
        port_write_8(address, data);
    }
}
