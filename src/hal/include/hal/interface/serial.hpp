#ifndef HAL_SERIAL_HPP
#define HAL_SERIAL_HPP

#include <kernel/types.hpp>
#include <stdint.h>

namespace a9n::hal
{
    void    init_serial(a9n::word baud_rate);
    uint8_t read_serial();
    void    write_serial(char data);
    void    write_string_serial(const char *out);
}

#endif
