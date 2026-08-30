#ifndef HAL_TIMER_HPP
#define HAL_TIMER_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>
#include <stdint.h>

namespace a9n::hal
{
    hal_result configure_system_clock_frequency(uint16_t hz);
}

#endif
