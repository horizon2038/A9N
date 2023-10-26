#include "pit_timer.hpp"

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

    pit_timer::pit_timer(hal::interface::port_io &injected_port_io)
        : _port_io(injected_port_io)
        , _pic(injected_port_io)
    {
        this_timer = this;
    }

    pit_timer::~pit_timer()
    {
    }

    void pit_timer::init_timer()
    {
        _pic.init_pic();
        configure_timer(100);
    }

    void pit_timer::configure_timer(uint16_t hz)
    {
        uint32_t pit_rate = CLOCK_RATE / hz;
        _port_io.write(PIT_COMMAND_REGISTER, 0x36);
        _port_io.write(PIT_CHANNEL_0, pit_rate & 0xff);
        _port_io.write(PIT_CHANNEL_0, pit_rate >> 8);
    }

    void pit_timer::clock()
    {
        ticks++;
        this_timer->_pic.end_of_interrupt_pic();
    }

    uint32_t pit_timer::get_tick()
    {
        return ticks;
    }

}
