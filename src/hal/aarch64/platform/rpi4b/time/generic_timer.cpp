#include <hal/aarch64/platform.hpp>
#include <hal/interface/timer.hpp>

namespace a9n::hal::aarch64::platform
{
    namespace
    {
        inline a9n::word timer_interval = 0;
    }

    hal_result init_system_timer()
    {
        a9n::word control = 0;
        asm volatile("msr cntp_ctl_el0, %0; isb" : : "r"(control) : "memory");
        return enable_irq(GENERIC_TIMER_IRQ);
    }

    void rearm_system_timer()
    {
        if (!timer_interval)
        {
            return;
        }
        asm volatile("msr cntp_tval_el0, %0; isb" : : "r"(timer_interval) : "memory");
    }

    hal_result configure_timer(uint16_t hz)
    {
        if (!hz)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        a9n::word frequency {};
        asm volatile("mrs %0, cntfrq_el0" : "=r"(frequency));
        timer_interval = frequency / hz;
        if (!timer_interval)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        rearm_system_timer();
        a9n::word control = 1;
        asm volatile("msr cntp_ctl_el0, %0; isb" : : "r"(control) : "memory");
        return {};
    }
}

namespace a9n::hal
{
    hal_result configure_system_clock_frequency(uint16_t hz)
    {
        return aarch64::platform::configure_timer(hz);
    }
}
