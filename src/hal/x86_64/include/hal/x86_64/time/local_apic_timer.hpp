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

      private:
        hal_result calibrate();

        uint32_t initial_count;
        uint8_t  divide_config;

        uint32_t frequency;
        uint32_t desired_frequency;
    };

    inline local_apic_timer local_apic_timer_core {};

}

#endif
