#include <hal/interface/port_io.hpp>

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> read_io_port(a9n::word, a9n::word)
    {
        return hal_error::UNSUPPORTED;
    }

    hal_result write_io_port(a9n::word, a9n::word, a9n::word)
    {
        return hal_error::UNSUPPORTED;
    }
}
