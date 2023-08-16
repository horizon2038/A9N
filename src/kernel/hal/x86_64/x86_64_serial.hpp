#ifndef X86_64_SERIAL_HPP
#define X86_64_SERIAL_HPP

#include <stdint.h>

namespace hal::x86_64
{
    class serial
    {
        public:
            void init_serial(uint32_t baud_rate);
            int read_serial();
            void write_serial();

        private:
            int isReceived();
            int isEmpty();

    };

constexpr static uint16_t COM_1 = 0x3f8;
constexpr static uint16_t COM_2 = 0x2f8;
constexpr static uint16_t COM_3 = 0x3e8;
constexpr static uint16_t COM_4 = 0x2e8;

}

#endif
