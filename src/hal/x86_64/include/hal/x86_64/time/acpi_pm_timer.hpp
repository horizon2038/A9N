#ifndef A9N_HAL_X86_64_TIME_ACPI_PM_TIMER_HPP
#define A9N_HAL_X86_64_TIME_ACPI_PM_TIMER_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/platform/acpi.hpp>

namespace a9n::hal::x86_64
{
    class acpi_pm_timer
    {
        // ACPI PM Timer frequency : 3.579545MHz
        static constexpr double TIMER_FREQUENCY = 3579545.0;

      public:
        hal_result                          init(fadt *fadt_base);
        liba9n::result<uint32_t, hal_error> read();
        hal_result                          wait(uint32_t micro_seconds);

      private:
        bool is_mmio { false };

        union
        {
            // ACPI v1, v2(revision <= 2)
            uint64_t port;
            // ACPI v2(revision > 2)
            volatile uint32_t *address;
        };

        uint8_t bits { 24 };
    };

    // WARN: global
    inline acpi_pm_timer acpi_pm_timer_core {};

}

#endif
