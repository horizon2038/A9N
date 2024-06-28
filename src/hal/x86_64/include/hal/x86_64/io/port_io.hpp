#ifndef X86_64_PORT_IO_HPP
#define X86_64_PORT_IO_HPP

#include <hal/interface/port_io.hpp>

namespace a9n::hal::x86_64
{

    class port_io final : public a9n::hal::port_io
    {
      public:
        uint8_t            read(uint16_t address) override;
        void               write(uint16_t address, uint8_t data) override;
        static inline void io_wait()
        {
            do
            {
            } while (0);
        }
    };
}

#endif
