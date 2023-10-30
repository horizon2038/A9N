#include "pit_timer.hpp"

#include "pic.hpp"

#include <library/logger.hpp>

namespace hal::x86_64
{
    namespace
    {
        constexpr uint32_t CLOCK_RATE = 1193182;
        constexpr uint16_t PIT_CHANNEL_0 = 0x40;
        constexpr uint16_t PIT_CHANNEL_1 = 0x41;
        constexpr uint16_t PIT_CHANNEL_2 = 0x42;
        constexpr uint16_t PIT_COMMAND_REGISTER = 0x43;
    }

    pit_timer *pit_timer::this_timer = nullptr;

    pit_timer::pit_timer()
        : _port_io()
    {
        this_timer = this;
    }

    pit_timer::~pit_timer()
    {
    }

    void pit_timer::init_timer()
    {
        kernel::utility::logger::printk("init timer\n");
        configure_timer(100);
        pic my_pic;
        my_pic.unmask_irq(0);
        my_pic.unmask_irq(4);
        my_pic.unmask_irq(5);
    }

    void pit_timer::configure_timer(uint16_t hz)
    {
        /*
        uint32_t pit_rate = CLOCK_RATE / hz;
        _port_io.write(PIT_COMMAND_REGISTER, 0x36);
        _port_io.write(PIT_CHANNEL_0, pit_rate & 0x0fe);
        _port_io.write(PIT_CHANNEL_0, pit_rate >> 8);
        */

        unsigned int divisor = CLOCK_RATE / hz;
        // _port_io.write(PIT_COMMAND_REGISTER, 0x36);  // Channel 0, lo/hi byte, rate generator
        _port_io.write(PIT_COMMAND_REGISTER, 0x36);  // Channel 0, lo/hi byte, rate generator
        _port_io.write(PIT_CHANNEL_0, divisor & 0xEF);
        // _port_io.write(PIT_CHANNEL_0, (divisor >> 8) & 0xFF);
        _port_io.write(PIT_CHANNEL_0, divisor >> 8);
    }

    void pit_timer::clock()
    {
        ticks++;
    }

    uint32_t pit_timer::get_tick()
    {
        return ticks;
    }

}
