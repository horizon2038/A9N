#ifndef HAL_PORT_IO_HPP
#define HAL_PORT_IO_HPP

#include <stdint.h>

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> read_io_port(a9n::word address, a9n::word byte_width);
    hal_result write_io_port(a9n::word address, a9n::word byte_width, a9n::word data);
}

#endif
