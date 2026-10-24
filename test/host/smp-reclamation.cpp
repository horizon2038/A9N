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
    bool reject_context_switch = false;
    kernel_entry_type entry_type = kernel_entry_type::INTERRUPT;
    unsigned restarted_syscalls = 0;
    pthread_mutex_t atomic_mutex = PTHREAD_MUTEX_INITIALIZER;

    template<typename Lock>
    concept accepts_runtime_guard_entry = requires(Lock &lock) {
        lock_guard<Lock>(lock, kernel_entry_type::SYSCALL);
    };

    struct plain_lock
    {
        bool held { false };

        kernel_result lock()
        {
            CHECK(!held);
            held = true;
            return {};
        }

        kernel_result unlock()
        {
            CHECK(held);
            held = false;
            return {};
        }
    };

    struct atomic_guard
    {
        atomic_guard() { CHECK(pthread_mutex_lock(&atomic_mutex) == 0); }
        ~atomic_guard() { CHECK(pthread_mutex_unlock(&atomic_mutex) == 0); }
    };

    template<kernel_entry_type Entry>
    void remote_entry_locked()
    {
        // Model a CPU that entered a syscall with IRQs masked before the stop
        // IPI. It must still service the request while waiting for giant_lock.
        lock_guard<giant_lock_type, Entry> guard(giant_lock);
        if (reject_context_switch)
        {
            CHECK(cpu_local_variables[1].current_process == &target);
            CHECK(!cpu_local_variables[1].is_idle);
        }
        else
        {
            CHECK(cpu_local_variables[1].current_process == &idle[1]);
            CHECK(cpu_local_variables[1].is_idle);
        }
    }

    void *remote_entry(void *)
    {
        core_number = 1;
        a9n::hal::atomic_store(&remote_entered, uint8_t { 1 });
        if (entry_type == kernel_entry_type::SYSCALL)
        {
            remote_entry_locked<kernel_entry_type::SYSCALL>();
        }
        else
        {
            remote_entry_locked<kernel_entry_type::INTERRUPT>();
        }
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
    void restart_syscall(process &process)
    {
        CHECK(cpu_local_variables[core_number].is_idle);
        process.registers[15] -= 2;
        ++restarted_syscalls;
    }
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
        if (reject_context_switch)
        {
            return kernel_error::UNEXPECTED;
        }
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
    static_assert(sizeof(spin_lock) == 64);
    static_assert(
        sizeof(lock_guard<giant_lock_type, kernel_entry_type::SYSCALL>) == sizeof(giant_lock_type *)
    );
    static_assert(!accepts_runtime_guard_entry<giant_lock_type>);
    plain_lock plain;
    {
        lock_guard guard(plain);
        CHECK(plain.held);
    }
    CHECK(!plain.held);
    null_lock no_op;
    {
        lock_guard<null_lock, kernel_entry_type::SYSCALL> guard(no_op);
    }
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
        target.registers[15] = 0x4002;
        entry_type = iteration % 2 == 0 ? kernel_entry_type::SYSCALL : kernel_entry_type::INTERRUPT;
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
        const auto expected_pc = entry_type == kernel_entry_type::SYSCALL ? 0x4000 : 0x4002;
        CHECK(target.registers[15] == expected_pc);
        CHECK(restarted_syscalls == iteration / 2 + 1);
        CHECK(stop_process_for_reclamation(target)); // stopped is idempotent
        CHECK(target.registers[15] == expected_pc);
        CHECK(giant_lock.unlock());
        CHECK(pthread_join(thread, nullptr) == 0);
    }
    // A failed context retirement must neither ACK success nor rewind the PC.
    // Retrying the same request rewinds exactly once, after the actual switch.
    const bool failures[] { true, false };
    target.registers[15] = 0x4002;
    for (bool fail_switch : failures)
    {
        core_number = 0;
        target.core_affinity = 1;
        remote.current_process = &target;
        remote.is_idle = false;
        target.status = process_status::READY;
        entry_type = kernel_entry_type::SYSCALL;
        reject_context_switch = fail_switch;
        a9n::hal::atomic_store(&remote_entered, uint8_t { 0 });
        CHECK(giant_lock.lock());
        pthread_t thread;
        CHECK(pthread_create(&thread, nullptr, remote_entry, nullptr) == 0);
        while (!a9n::hal::atomic_load(&remote_entered)) {}
        const auto result = stop_process_for_reclamation(target);
        CHECK(static_cast<bool>(result) == !fail_switch);
        CHECK(target.registers[15] == (fail_switch ? 0x4002 : 0x4000));
        CHECK(giant_lock.unlock());
        CHECK(pthread_join(thread, nullptr) == 0);
    }
    target.core_affinity = 0;
    cpu_local_variables[0].current_process = &target;
    cpu_local_variables[0].is_idle = false;
    CHECK(stop_process_for_reclamation(target));
    CHECK(cpu_local_variables[0].current_process == &idle[0]);
    puts("SMP reclamation: 100 IRQ-masked entries, PC replay, ACK ordering, failed IPI/context switch retry and local stop PASS");
}
