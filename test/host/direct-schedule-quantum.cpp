#include <kernel/process/cpu.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/utility/logger.hpp>
#include <stdio.h>
#include <stdlib.h>

using namespace a9n::kernel;

#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); abort(); \
} } while (0)

namespace
{
    cpu_local_variable *active_local;
    process *switched_from, *switched_to;
    unsigned switches;

    struct fixture
    {
        process current {}, target {};
        cpu_local_variable local {};

        fixture()
        {
            current.status = target.status = process_status::READY;
            current.priority = target.priority = 23;
            current.core_affinity = target.core_affinity = BSP_ID;
            current.quantum = 7; // Partially consumed: self-switch must not reset it either.
            target.quantum = 5;
            local.core_number = BSP_ID;
            local.current_process = &current;
            active_local = &local;
            switches = 0;
            switched_from = switched_to = nullptr;
        }

        ~fixture() { active_local = nullptr; }
    };
}

namespace a9n::hal
{
    liba9n::result<cpu_local_variable *, hal_error> current_local_variable()
    {
        CHECK(active_local);
        return active_local;
    }

    hal_result switch_context(process &previous, process &next)
    {
        ++switches;
        switched_from = &previous;
        switched_to = &next;
        return {};
    }
}

namespace a9n::kernel::utility
{
    void logger::printk(const char *, ...) {}
    void logger::printh(const char *, ...) {}
    void logger::error(const char *) {}
}

static void self_switch()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    for (unsigned i = 0; i < 16; ++i)
    {
        // Exercise both overloads, including the HAL local-variable lookup.
        if (i & 1) CHECK(manager.try_direct_schedule_and_switch(f.current));
        else CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
        CHECK(f.current.quantum == 7);
        CHECK(f.local.current_process == &f.current && !f.local.is_idle);
        CHECK(!f.current.is_in_ready_queue);
        CHECK(switched_from == &f.current && switched_to == &f.current);
    }
    CHECK(switches == 16);
}

static void switch_to_other()
{
    for (unsigned overload = 0; overload < 2; ++overload)
    {
        fixture f;
        auto &manager = f.local.process_manager_core;
        if (overload) CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
        else CHECK(manager.try_direct_schedule_and_switch(f.target));
        CHECK(f.current.quantum == 7 && f.target.quantum == 5);
        CHECK(f.local.current_process == &f.target && !f.local.is_idle);
        CHECK(switched_from == &f.current && switched_to == &f.target && switches == 1);
    }
}

static void self_request_selects_higher_priority()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    f.target.priority = 28;
    CHECK(manager.mark_scheduled(f.target));
    f.target.quantum = 5;

    // A request to continue the current process can still select another process.
    CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
    CHECK(f.local.current_process == &f.target);
    CHECK(f.current.is_in_ready_queue && !f.target.is_in_ready_queue);
    CHECK(f.current.quantum == 7 && f.target.quantum == 5);
    CHECK(switched_from == &f.current && switched_to == &f.target && switches == 1);
}

static void timer_still_expires_after_self_switches()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    f.current.quantum = 3;
    CHECK(manager.mark_scheduled(f.target)); // Same priority, waiting for its turn.

    for (a9n::sword remaining = 3; remaining > 0; --remaining)
    {
        for (unsigned i = 0; i < 16; ++i)
        {
            CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
            CHECK(f.current.quantum == remaining);
        }
        CHECK(manager.handle_timer());
        if (remaining > 1) CHECK(f.local.current_process == &f.current);
    }
    CHECK(f.local.current_process == &f.target);
    CHECK(f.current.quantum == QUANTUM_MAX && f.current.is_in_ready_queue);
    CHECK(!f.target.is_in_ready_queue);
}

static void rejected_target_preserves_quantum()
{
    fixture f;
    f.target.priority = -1;
    CHECK(!f.local.process_manager_core.try_direct_schedule_and_switch(f.target, f.local));
    CHECK(f.current.quantum == 7 && f.target.quantum == 5);
    CHECK(f.local.current_process == &f.current && switches == 0);
}

static void repeated_ipc_preserves_quantum()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    // Model call -> server -> fast reply-receive -> caller. Neither path
    // donates or refreshes quantum; each process keeps its remaining budget.
    for (unsigned i = 0; i < 1000; ++i)
    {
        CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
        CHECK(f.current.quantum == 7 && f.target.quantum == 5);
        CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
        CHECK(f.current.quantum == 7 && f.target.quantum == 5);
    }
    CHECK(manager.mark_scheduled(f.current)); // A peer must eventually run.
    CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
    for (unsigned tick = 0; tick < 5; ++tick)
    {
        CHECK(manager.handle_timer());
        if (f.local.current_process == &f.current) return;
    }
    CHECK(false);
}

static void continuous_ipc_does_not_replenish_server()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    process peer {};
    peer.status = process_status::READY;
    peer.priority = f.target.priority;
    peer.core_affinity = BSP_ID;
    CHECK(manager.mark_scheduled(peer));

    for (a9n::sword remaining = 5; remaining > 0; --remaining)
    {
        // Continue calling between every pair of timer ticks, not merely
        // before a burst ends. Model timer interrupts during server execution.
        for (unsigned call = 0; call < 64; ++call)
        {
            f.current.status = process_status::BLOCKED_REPLY;
            f.target.status = process_status::READY;
            CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
            CHECK(f.target.quantum == remaining);
            f.target.status = process_status::BLOCKED_RECEIVE;
            f.current.status = process_status::READY;
            CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
            CHECK(f.target.quantum == remaining && f.current.quantum == 7);
        }
        f.current.status = process_status::BLOCKED_REPLY;
        f.target.status = process_status::READY;
        CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
        CHECK(manager.handle_timer());
        if (remaining == 1)
        {
            CHECK(f.local.current_process == &peer);
            CHECK(f.target.is_in_ready_queue && f.target.quantum == QUANTUM_MAX);
            return;
        }
        CHECK(f.local.current_process == &f.target);
        CHECK(f.target.quantum == remaining - 1);
        f.target.status = process_status::BLOCKED_RECEIVE;
        f.current.status = process_status::READY;
        CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
    }
    CHECK(false);
}

static void continuous_ipc_does_not_replenish_caller()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    process peer {};
    peer.status = process_status::READY;
    peer.priority = f.current.priority;
    peer.core_affinity = BSP_ID;
    CHECK(manager.mark_scheduled(peer));

    for (a9n::sword remaining = 7; remaining > 0; --remaining)
    {
        // Calls continue between ticks that all fall on the caller, rather
        // than the server. Fast Reply-Receive must not replenish its budget.
        for (unsigned call = 0; call < 64; ++call)
        {
            f.current.status = process_status::BLOCKED_REPLY;
            f.target.status = process_status::READY;
            CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
            CHECK(f.current.quantum == remaining && f.target.quantum == 5);
            f.target.status = process_status::BLOCKED_RECEIVE;
            f.current.status = process_status::READY;
            CHECK(manager.try_direct_schedule_and_switch(f.current, f.local));
            CHECK(f.current.quantum == remaining && f.target.quantum == 5);
        }
        CHECK(manager.handle_timer());
        if (remaining == 1)
        {
            CHECK(f.local.current_process == &peer);
            CHECK(f.current.is_in_ready_queue && f.current.quantum == QUANTUM_MAX);
            return;
        }
        CHECK(f.local.current_process == &f.current);
        CHECK(f.current.quantum == remaining - 1);
    }
    CHECK(false);
}

static void exhausted_quantum_is_not_refilled_by_direct_switch()
{
    fixture f;
    auto &manager = f.local.process_manager_core;
    f.target.quantum = 0;
    CHECK(manager.mark_scheduled(f.current));
    f.current.quantum = 7;
    CHECK(manager.try_direct_schedule_and_switch(f.target, f.local));
    CHECK(f.current.quantum == 7 && f.target.quantum == 0);
    CHECK(manager.handle_timer());
    CHECK(f.local.current_process == &f.current);
    CHECK(f.target.quantum == QUANTUM_MAX && f.target.is_in_ready_queue);
}

int main()
{
    self_switch();
    switch_to_other();
    self_request_selects_higher_priority();
    timer_still_expires_after_self_switches();
    rejected_target_preserves_quantum();
    repeated_ipc_preserves_quantum();
    continuous_ipc_does_not_replenish_server();
    continuous_ipc_does_not_replenish_caller();
    exhausted_quantum_is_not_refilled_by_direct_switch();
    puts("PASS: 9 direct-schedule quantum scenarios without donation (production process manager and scheduler)");
}
