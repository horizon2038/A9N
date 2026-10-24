#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/interrupt/irq_notification_handlers.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/process/lock.hpp>
#include <kernel/utility/logger.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>
#include <stdio.h>
#include <stdlib.h>

using namespace a9n::kernel;

using a9n::hal::x86_64::reserved_irq;
static_assert(static_cast<unsigned>(reserved_irq::BASE) == 0x20);
static_assert(static_cast<unsigned>(reserved_irq::TIMER) == 0x20);
static_assert(static_cast<unsigned>(reserved_irq::IO_BASE) == 0x30);
static_assert(static_cast<unsigned>(reserved_irq::IO_BASE) + 16 == 0x40);
static_assert(static_cast<unsigned>(reserved_irq::IO_BASE) + 23
              < static_cast<unsigned>(reserved_irq::IPI_HALT));

#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); abort(); \
} } while (0)

namespace
{
    process_manager manager {};
    process current {};
    notification_port port {};
    bool manager_error, process_error, notify_error, mask_error, eoi_error;
    unsigned notified, completed, order[2];
    a9n::word expected_irq;
}

namespace a9n::kernel
{
    void spin_lock::lock_contended(kernel_entry_type) { abort(); }
    liba9n::result<process_manager *, kernel_error> current_process_manager()
    {
        if (manager_error) return kernel_error::TRY_AGAIN;
        return &manager;
    }
    liba9n::result<process *, kernel_error> process_manager::retrieve_current_process()
    {
        if (process_error) return kernel_error::TRY_AGAIN;
        return &current;
    }
    capability_result notification_port::execute(process &, capability_slot &) { abort(); }
    capability_result notification_port::revoke(capability_slot &) { abort(); }
    capability_result notification_port::operation_notify(process &, capability_slot &)
    {
        ++notified;
        if (notify_error) return capability_error::FATAL;
        return {};
    }
}
namespace a9n::kernel::utility
{
    void logger::printk(const char *, ...) {}
    void logger::printh(const char *, ...) {}
}
namespace a9n::hal
{
    hal_result disable_interrupt(a9n::word irq)
    {
        CHECK(irq == expected_irq && completed < 2);
        order[completed++] = 1;
        if (mask_error) return hal_error::TRY_AGAIN;
        return {};
    }
    hal_result ack_interrupt()
    {
        CHECK(completed < 2);
        order[completed++] = 2;
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

static void completion_tests(a9n::word irq)
{
    const unsigned scenarios[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    for (unsigned scenario : scenarios)
    {
        manager_error = scenario == 1;
        process_error = scenario == 2;
        notify_error = scenario == 3;
        mask_error = scenario == 8;
        eoi_error = scenario == 9;
        notified = completed = 0;
        expected_irq = scenario == 7 ? a9n::hal::IRQ_NUMBER_MAX : irq;
        auto &handler = irq_notification_handlers[irq];
        handler.irq_number = irq;
        CHECK(try_configure_notification_port_slot(handler.slot, port, 1));
        if (scenario == 4) handler.slot.type = capability_type::NONE;
        if (scenario == 5) handler.slot.type = capability_type::FRAME;
        if (scenario == 6) handler.slot.component = nullptr;
        handle_interrupt(expected_irq);
        CHECK(completed == 2);
        CHECK(order[0] == 1 && order[1] == 2); // Mask before EOI, including errors.
        CHECK(notified == (scenario == 0 || scenario == 3 || scenario >= 8 ? 1u : 0u));
    }
}

int main()
{
    const a9n::word irqs[] { 0, 12, 16, 23 };
    for (a9n::word irq : irqs)
    {
        completion_tests(irq);
    }
    puts("PASS: fixed timer/IO vectors and 40 external IRQ completion paths (production handler)");
}
