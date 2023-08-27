#include "pit_timer.hpp"

namespace hal::x86_64
{
    constexpr uint32_t CLOCK_RATE = 1193810;
    constexpr uint16_t PIT_CHANNEL_0 = 0x40;
    constexpr uint16_t PIT_CHANNEL_1 = 0x41;
    constexpr uint16_t PIT_CHANNEL_2 = 0x42;
    constexpr uint16_t PIT_COMMAND_REGISTER = 0x43;

    pit_timer::pit_timer(hal::interface::port_io &injected_port_io, hal::interface::serial &injected_serial)
        : _port_io(injected_port_io),
          _serial(injected_serial)
    {
    }
    
    pit_timer::~pit_timer()
    {
    }

    void pit_timer::init_timer()
    {
        configure_timer(1000);
    }

    void pit_timer::configure_timer(uint16_t hz)
    {
        uint16_t pit_rate = CLOCK_RATE / hz;
        _port_io.write(PIT_COMMAND_REGISTER, 0x36);
        _port_io.write(PIT_CHANNEL_0, pit_rate & 0xff);
        _port_io.write(PIT_CHANNEL_0, pit_rate >> 8);
    }

    __attribute__((interrupt)) void pit_timer::handle()
    {
        ticks++;
        _serial.write_serial((char)ticks);
    }
}
