#ifndef A9N_HAL_X86_64_LOCAL_APIC_TIMER_HPP
#define A9N_HAL_X86_64_LOCAL_APIC_TIMER_HPP

#include <hal/hal_result.hpp>
#include <hal/interface/timer.hpp>

namespace a9n::hal::x86_64
{
    class local_apic_timer final : public a9n::hal::timer
    {
      public:
        hal_result init() override;
        hal_result configure_cycle(uint16_t hz) override;
        hal_result configure_system_clock_frequency(uint16_t hz);
        hal_result init_current_core();

      private:
        hal_result calibrate();
        hal_result enable_current_core();

        uint32_t frequency { 0 };
        uint16_t desired_frequency { 0 };
        uint8_t  divide_config { 0 };
    };

    inline local_apic_timer local_apic_timer_core {};

}

#endif
