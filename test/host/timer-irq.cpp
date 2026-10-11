#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/process/process_manager.hpp>
#include <stdio.h>
#include <stdlib.h>

using namespace a9n::kernel;

#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); abort(); \
} } while (0)

namespace
{
    process_manager manager {};
    bool manager_error, timer_error, eoi_error;
    unsigned calls, order[3];

    void record(unsigned operation)
    {
        CHECK(calls < 3);
        order[calls++] = operation;
    }
}

namespace a9n::kernel
{
    liba9n::result<process_manager *, kernel_error> current_process_manager()
    {
        record(1);
        if (manager_error) return kernel_error::TRY_AGAIN;
        return &manager;
    }

    kernel_result process_manager::handle_timer()
    {
        record(2);
        if (timer_error) return kernel_error::TRY_AGAIN;
        return {};
    }
}

namespace a9n::hal
{
    hal_result ack_interrupt()
    {
        record(3);
        if (eoi_error) return hal_error::TRY_AGAIN;
        return {};
    }

    uint32_t atomic_compare_exchange(volatile uint32_t *address, uint32_t expected, uint32_t desired)
    {
        auto previous = *address;
        if (previous == expected) *address = desired;
        return previous;
    }
    void atomic_store(volatile uint32_t *address, uint32_t value) { *address = value; }
    void spin_wait() { abort(); }
}

int main()
{
    for (unsigned scenario = 0; scenario < 8; ++scenario)
    {
        manager_error = (scenario & 1) != 0;
        timer_error = (scenario & 2) != 0;
        eoi_error = (scenario & 4) != 0;
        calls = 0;

        handle_timer();

        CHECK(calls == (manager_error ? 2u : 3u));
        CHECK(order[0] == 1);
        if (!manager_error) CHECK(order[1] == 2);
        CHECK(order[calls - 1] == 3); // Exactly one EOI attempt, after timer processing.
    }
    puts("PASS: 8 timer IRQ completion paths, including lookup/handler/EOI errors (production handler)");
}
