#include <hal/aarch64/platform.hpp>
#include <hal/interface/timer.hpp>
#include <kernel/utility/logger.hpp>

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
        a9n::word frequency {};
        a9n::word mpidr {};
        asm volatile("mrs %0, cntfrq_el0" : "=r"(frequency));
        asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
        a9n::kernel::utility::logger::printh(
            "Generic timer initialized: MPIDR=0x%016llx, frequency=%llu Hz, PPI=%llu\n",
            mpidr,
            frequency,
            GENERIC_TIMER_IRQ
        );
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

        a9n::kernel::utility::logger::printh(
            "Generic timer configured: frequency=%llu Hz, tick-rate=%llu Hz, interval=%llu\n",
            frequency,
            static_cast<a9n::word>(hz),
            timer_interval
        );

        return start_system_timer();
    }

    hal_result start_system_timer()
    {
        if (!timer_interval)
        {
            return hal_error::INIT_FIRST;
        }
        rearm_system_timer();
        a9n::word control = 1;
        asm volatile("msr cntp_ctl_el0, %0; isb" : : "r"(control) : "memory");
        a9n::word mpidr {};
        asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
        a9n::kernel::utility::logger::printh(
            "Generic timer started: MPIDR=0x%016llx, interval=%llu\n",
            mpidr,
            timer_interval
        );
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
