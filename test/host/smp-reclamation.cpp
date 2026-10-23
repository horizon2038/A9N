#include <kernel/process/lock.hpp>
#include <kernel/process/reclamation.hpp>
#include <hal/interface/interrupt.hpp>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

using namespace a9n::kernel;
#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); abort(); \
} } while (0)

namespace
{
    thread_local unsigned core_number = 0;
    process idle[2] {};
    process target {};
    volatile uint8_t remote_entered = 0;
    unsigned sent_ipis = 0;
    unsigned parked = 0;
    bool reject_ipi = false;
    pthread_mutex_t atomic_mutex = PTHREAD_MUTEX_INITIALIZER;

    struct atomic_guard
    {
        atomic_guard() { CHECK(pthread_mutex_lock(&atomic_mutex) == 0); }
        ~atomic_guard() { CHECK(pthread_mutex_unlock(&atomic_mutex) == 0); }
    };

    void *remote_entry(void *)
    {
        core_number = 1;
        a9n::hal::atomic_store(&remote_entered, uint8_t { 1 });
        // Model a CPU that entered a syscall with IRQs masked before the stop
        // IPI. It must still service the request while waiting for giant_lock.
        CHECK(giant_lock.lock());
        CHECK(cpu_local_variables[1].current_process == &idle[1]);
        CHECK(cpu_local_variables[1].is_idle);
        CHECK(giant_lock.unlock());
        return nullptr;
    }
}

namespace a9n::hal
{
    liba9n::result<cpu_local_variable *, hal_error> current_local_variable()
    {
        return &cpu_local_variables[core_number];
    }
    hal_result send_ipi(ipi_type type, a9n::word destination)
    {
        CHECK(type == ipi_type::RESCHEDULE && destination == 1);
        if (reject_ipi)
        {
            return hal_error::UNEXPECTED;
        }
        ++sent_ipis;
        return {};
    }
    uint32_t atomic_compare_exchange(volatile uint32_t *address, uint32_t expected, uint32_t desired)
    {
        atomic_guard guard;
        const auto previous = *address;
        if (previous == expected)
        {
            *address = desired;
        }
        return previous;
    }
    uint8_t atomic_load(const volatile uint8_t *address)
    {
        atomic_guard guard;
        return *address;
    }
    void atomic_store(volatile uint8_t *address, uint8_t value)
    {
        atomic_guard guard;
        *address = value;
    }
    void atomic_store(volatile uint32_t *address, uint32_t value)
    {
        atomic_guard guard;
        *address = value;
    }
    void spin_wait() {}
}

namespace a9n::kernel
{
    kernel_result process_manager::mark_suspended(process &process)
    {
        process.status = process_status::BLOCKED_SUSPEND;
        process.is_in_ready_queue = false;
        return {};
    }
    kernel_result process_manager::switch_to_idle()
    {
        auto &local = cpu_local_variables[core_number];
        CHECK(local.current_process == &target);
        CHECK(target.status == process_status::BLOCKED_SUSPEND);
        // The real implementation saves FPU/TLS and changes the page table
        // before publishing IDLE. The production mailbox must ACK afterwards.
        local.current_process = &idle[core_number];
        local.is_idle = true;
        ++parked;
        return {};
    }
}

int main()
{
    static_assert(sizeof(spin_lock_no_owner) == 64);
    target.core_affinity = 1;
    auto &remote = cpu_local_variables[1];
    remote.core_number = 1;
    remote.current_process = &target;
    remote.is_idle = false;
    CHECK(giant_lock.lock());
    reject_ipi = true;
    CHECK(!stop_process_for_reclamation(target));
    CHECK(remote.current_process == &target && !remote.is_idle);
    reject_ipi = false;
    CHECK(giant_lock.unlock());
    // The failed send must not leave a published request; the following
    // iterations retry the same target through a real contended entry.
    for (unsigned iteration = 0; iteration < 100; ++iteration)
    {
        core_number = 0;
        target.core_affinity = 1;
        target.status = process_status::READY;
        target.is_in_ready_queue = true;
        auto &remote = cpu_local_variables[1];
        remote.core_number = 1;
        remote.current_process = &target;
        remote.is_idle = false;
        remote.pending_reschedule_target = &target;
        a9n::hal::atomic_store(&remote_entered, uint8_t { 0 });
        CHECK(giant_lock.lock());
        pthread_t thread;
        CHECK(pthread_create(&thread, nullptr, remote_entry, nullptr) == 0);
        while (!a9n::hal::atomic_load(&remote_entered)) {}
        CHECK(stop_process_for_reclamation(target));
        CHECK(remote.current_process == &idle[1] && remote.is_idle);
        CHECK(!remote.pending_reschedule_target && !target.is_in_ready_queue);
        CHECK(parked == iteration + 1 && sent_ipis == iteration + 1);
        CHECK(stop_process_for_reclamation(target)); // stopped is idempotent
        CHECK(giant_lock.unlock());
        CHECK(pthread_join(thread, nullptr) == 0);
    }
    target.core_affinity = 0;
    cpu_local_variables[0].current_process = &target;
    cpu_local_variables[0].is_idle = false;
    CHECK(stop_process_for_reclamation(target));
    CHECK(cpu_local_variables[0].current_process == &idle[0]);
    puts("SMP reclamation: 100 IRQ-masked entries, ACK ordering, failed IPI retry and local stop PASS");
}
