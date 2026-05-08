#include <hal/x86_64/time/pit_timer.hpp>

#include <hal/x86_64/interrupt/pic.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    namespace
    {
        constexpr uint32_t CLOCK_RATE           = 1193182;
        constexpr uint16_t PIT_CHANNEL_0        = 0x40;
        constexpr uint16_t PIT_CHANNEL_1        = 0x41;
        constexpr uint16_t PIT_CHANNEL_2        = 0x42;
        constexpr uint16_t PIT_COMMAND_REGISTER = 0x43;
    }

    hal_result pit_timer::init()
    {
        a9n::kernel::utility::logger::printk("init timer\n");
        configure_cycle(100);
        pic my_pic;
        my_pic.unmask_irq(0);
        my_pic.unmask_irq(4);

        return {};
    }

    hal_result pit_timer::configure_cycle(uint16_t hz)
    {
        unsigned int divisor = CLOCK_RATE / hz;

        // byte, rate generator
        _port_io.write(
            PIT_COMMAND_REGISTER,
            0x36
        ); // Channel 0, lo/hi byte, rate generator

        _port_io.write(PIT_CHANNEL_0, divisor & 0xEF);
        _port_io.write(PIT_CHANNEL_0, divisor >> 8);

        return {};
    }

}
