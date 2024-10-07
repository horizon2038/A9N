#include <hal/x86_64/time/local_apic_timer.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/interrupt/apic.hpp>
#include <hal/x86_64/platform/acpi.hpp>
#include <hal/x86_64/time/acpi_pm_timer.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    hal_result result_none()
    {
        return hal_result {};
    }

    hal_result local_apic_timer::init()
    {
        a9n::kernel::utility::logger::printk("init Local APIC Timer\n");
        divide_config = 0x03;

        return calibrate()
            .and_then(
                [this](void) -> hal_result
                {
                    // TODO : stop using magic number
                    return configure_cycle(250);
                }
            )
            .and_then(
                [this](void) -> hal_result
                {
                    // configure irq
                    return local_apic_core
                        .write(local_apic_offset::LVT_TIMER, 0x20 | (1 << 17));
                }
            )
            .or_else(
                [this](hal_error e) -> hal_result
                {
                    a9n::kernel::utility::logger::printk(
                        "failed to initialize "
                        "Local APIC Timer\n"
                    );
                    return e;
                }
            );
    }

    hal_result local_apic_timer::calibrate(void)
    {
        // mask Local APIC Timer
        return local_apic_core.write(local_apic_offset::LVT_TIMER, (1 << 16))
            .and_then(
                [&, this](void) -> hal_result
                {
                    // clear Local APIC Timer
                    a9n::kernel::utility::logger::printk(
                        "clear Local APIC "
                        "Timer\n"
                    );
                    return local_apic_core
                        .write(local_apic_offset::TIMER_INIT_COUNT, 0xFFFFFFFF);
                }
            )
            .and_then(
                [&, this](void) -> hal_result
                {
                    // configure divide
                    a9n::kernel::utility::logger::printk("configure divide\n");
                    return local_apic_core.write(
                        local_apic_offset::TIMER_DIVIDE,
                        divide_config & 0x0F
                    );
                }
            )
            .and_then(
                [&, this](void) -> hal_result
                {
                    // start Local APIC Timer
                    a9n::kernel::utility::logger::printk(
                        "start Local APIC "
                        "Timer measurement ...\n"
                    );
                    return local_apic_core
                        .write(local_apic_offset::TIMER_INIT_COUNT, 0xFFFFFFFF);
                }
            )
            .and_then(
                [&, this](void) -> hal_result
                {
                    // wait 10ms
                    return acpi_pm_timer_core.wait(10000); // micro_seconds unit
                }
            )
            .and_then(
                [&, this](void) -> liba9n::result<uint32_t, hal_error>
                {
                    return local_apic_core.read(local_apic_offset::TIMER_CURRENT_COUNT
                    );
                }
            )
            .and_then(
                [&, this](uint32_t start_clock) -> hal_result
                {
                    // calculate Local APIC Timer frequency
                    a9n::kernel::utility::logger::printk(
                        "calculate Local APIC "
                        "Timer frequency\n"
                    );
                    uint32_t elapsed_clock = 0xFFFFFFFF - start_clock;
                    frequency              = (elapsed_clock * 100);

                    a9n::kernel::utility::logger::printk(
                        "elapsed_clock : %8llu times\n",
                        elapsed_clock
                    );

                    a9n::kernel::utility::logger::printk(
                        "Local APIC Timer frequency : %10llu Hz ( %4llu MHz "
                        ")\n",
                        frequency,
                        frequency / 1000000
                    );

                    return hal_result {};
                }
            );
    }

    hal_result local_apic_timer::configure_cycle(uint16_t hz)
    {
        using a9n::kernel::utility::logger;

        if (frequency == 0)
        {
            return hal_error::INIT_FIRST;
        }

        auto initial_count = frequency / hz;

        return local_apic_core
            .write(local_apic_offset::TIMER_DIVIDE, divide_config & 0x0F)
            .and_then(
                [&, this](void) -> hal_result
                {
                    // configure divide
                    return local_apic_core.write(
                        local_apic_offset::TIMER_INIT_COUNT,
                        initial_count
                    );
                }
            )
            .or_else(
                [&, this](hal_error e) -> hal_result
                {
                    a9n::kernel::utility::logger::printk(
                        "configure_cycle "
                        "failed\n"
                    );
                    return e;
                }
            );
    }
}
