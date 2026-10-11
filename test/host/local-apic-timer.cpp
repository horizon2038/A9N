#include <hal/x86_64/time/local_apic_timer.hpp>
#include <hal/x86_64/time/acpi_pm_timer.hpp>
#include <hal/x86_64/interrupt/apic.hpp>
#include <kernel/utility/logger.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(expression) do { if (!(expression)) { \
    std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); std::exit(1); \
} } while (0)

namespace
{
    constexpr unsigned MASK = 1u << 16, PERIODIC = 1u << 17;
    unsigned lvt = MASK, initial = 0, current = 0, writes = 0;
    unsigned order[16] {}, fail_offset = 0;
    bool delay_after_count = false, running = false;

    void advance(unsigned cycles)
    {
        if (!running) return;
        if (cycles < current) { current -= cycles; return; }
        if (lvt & PERIODIC) current = initial - (cycles - current) % initial;
        else { current = 0; running = false; }
    }

    void reset(bool delay)
    {
        lvt = MASK;
        initial = current = writes = fail_offset = 0;
        running = false;
        delay_after_count = delay;
    }

    void check_periodic()
    {
        using namespace a9n::hal::x86_64;
        CHECK(running && current != 0 && initial == 6001);
        CHECK(lvt == (PERIODIC | 0x20));
        CHECK(writes == 3);
        CHECK(order[0] == local_apic_offset::TIMER_DIVIDE);
        CHECK(order[1] == local_apic_offset::LVT_TIMER);
        CHECK(order[2] == local_apic_offset::TIMER_INIT_COUNT);
        advance(initial * 3 + 1);
        CHECK(running && current != 0);
    }
}

namespace a9n::hal::x86_64
{
    hal_result local_apic::write(uint32_t offset, uint64_t value)
    {
        CHECK(writes < 16);
        order[writes++] = offset;
        if (offset == fail_offset) return hal_error::TRY_AGAIN;
        if (offset == local_apic_offset::LVT_TIMER) lvt = value;
        if (offset == local_apic_offset::TIMER_DIVIDE) CHECK(value == 3);
        if (offset == local_apic_offset::TIMER_INIT_COUNT)
        {
            initial = current = value;
            running = initial != 0;
            // Model elapsed hardware time between MMIO writes, e.g. an SMI.
            // Changing LVT mode cannot restart an expired one-shot counter.
            if (delay_after_count) advance(initial + 1);
        }
        return {};
    }

    liba9n::result<uint32_t, hal_error> local_apic::read(uint32_t offset)
    {
        CHECK(offset == local_apic_offset::TIMER_CURRENT_COUNT);
        return current;
    }

    hal_result acpi_pm_timer::wait(uint32_t microseconds)
    {
        CHECK(microseconds == 10000);
        advance(15003); // Reproduce the physical machine's 1,500,300 Hz calibration.
        return {};
    }
}

namespace a9n::kernel::utility
{
    void logger::printh(const char *, ...) {}
    void logger::error(const char *) {}
}

int main()
{
    using namespace a9n::hal::x86_64;
    local_apic_timer timer;
    CHECK(!timer.configure_system_clock_frequency(250));
    CHECK(!timer.init_current_core());
    CHECK(writes == 0);
    CHECK(timer.init());
    reset(false);
    CHECK(!timer.configure_system_clock_frequency(0));
    CHECK(writes == 0);

    // Both entry points use the production initialization code. No interrupt
    // handler or scheduler is simulated; this tests register programming only.
    const bool delays[] { true, false };
    for (bool delay : delays)
    {
        reset(delay);
        CHECK(timer.configure_system_clock_frequency(250));
        check_periodic();
        reset(delay);
        CHECK(timer.init_current_core());
        check_periodic();
    }

    const unsigned offsets[] { local_apic_offset::TIMER_DIVIDE,
                               local_apic_offset::LVT_TIMER,
                               local_apic_offset::TIMER_INIT_COUNT };
    for (unsigned i = 0; i < 3; ++i)
    {
        reset(false);
        fail_offset = offsets[i];
        CHECK(!timer.configure_system_clock_frequency(100));
        CHECK(writes == i + 1 && !running);
        reset(false);
        CHECK(timer.init_current_core()); // Failed setup must not replace the saved 250 Hz.
        check_periodic();
    }
    std::puts("PASS: BSP/AP timer order, delayed count write, validation and MMIO errors");
}
