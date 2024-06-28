#ifndef HAL_SERIAL_HPP
#define HAL_SERIAL_HPP

#include <stdint.h>
#include <library/common/types.hpp>

namespace a9n::hal
{
    class serial
    {
      public:
        virtual void    init_serial(a9n::word baud_rate) = 0;
        virtual uint8_t read_serial()                       = 0;
        virtual void    write_serial(char data)             = 0;
        virtual void    write_string_serial(char *out)      = 0;
    };
}

#endif
