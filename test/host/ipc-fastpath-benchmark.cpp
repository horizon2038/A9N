// Production CALL/Reply-Receive; scheduling and privileged HAL are substituted.
// This measures host C++ overhead, not syscall/context-switch latency.
#include <kernel/capability/ipc_port.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/utility/logger.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

using namespace a9n::kernel;

namespace
{
    a9n::word switches = 0;
    a9n::word calls = 0;
    a9n::word checksum = 0;

    struct participant
    {
        process core {};
        ipc_buffer buffer {};
        participant()
        {
            core.status = process_status::READY;
            core.buffer = &buffer;
            core.quantum = 7;
        }
    };

    uint64_t now()
    {
        timespec time {};
        if (clock_gettime(CLOCK_MONOTONIC, &time) != 0)
        {
            abort();
        }
        return static_cast<uint64_t>(time.tv_sec) * 1000000000 + time.tv_nsec;
    }

    void invoke(ipc_port &port, capability_slot &slot, participant &target, a9n::word operation)
    {
        target.buffer.messages[1] = operation;
        target.buffer.messages[2] = 3; // blocking, one payload word
        target.buffer.messages[3] = 0;
        if (!port.execute(target.core, slot))
        {
            abort();
        }
    }
}

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> get_message_register(const process &target, a9n::word index)
    {
        return target.buffer->messages[index];
    }
    hal_result configure_message_register(process &target, a9n::word index, a9n::word value)
    {
        target.buffer->messages[index] = value;
        return {};
    }
    liba9n::result<a9n::word, hal_error> get_general_register(const process &, register_type)
    {
        abort();
    }
}

namespace a9n::kernel
{
    kernel_result try_schedule_and_switch(process &) { return {}; }
    kernel_result mark_scheduled(process &, process &target)
    {
        target.status = process_status::READY;
        return {};
    }
    kernel_result mark_scheduled_with_preemption(process &owner, process &target)
    {
        return mark_scheduled(owner, target);
    }
    kernel_result try_direct_schedule_and_switch(process &, process &) { ++switches; return {}; }
    kernel_result process_manager::try_direct_schedule_and_switch(process &, cpu_local_variable &)
    {
        ++switches;
        return {};
    }
}

namespace a9n::kernel::utility
{
    void logger::printk(const char *, ...) {}
    void logger::printh(const char *, ...) {}
    void logger::error(const char *) { abort(); }
}

int main(int argc, char **argv)
{
    const unsigned samples = 9;
    const a9n::word iterations = 200000;
    printf("%s sizeof(process)=%zu sizeof(ipc_port)=%zu reply-target-offset=%zu\n",
           argc > 1 ? argv[1] : "current", sizeof(process), sizeof(ipc_port),
           offsetof(process, source_reply_target));
    participant server, caller;
    ipc_port endpoint;
    capability_slot slot {};
    slot.rights = capability_slot::ALL;
    invoke(endpoint, slot, server, 2);
    for (unsigned sample = 0; sample < samples; ++sample)
    {
        const auto start = now();
        for (a9n::word iteration = 0; iteration < iterations; ++iteration)
        {
            caller.buffer.messages[4] = iteration;
            invoke(endpoint, slot, caller, 3);
            server.buffer.messages[4] += 1;
            invoke(endpoint, slot, server, 5);
            checksum += caller.buffer.messages[4];
            ++calls;
        }
        printf("%s smp=%u sample=%u ns/roundtrip=%.3f\n", argc > 1 ? argv[1] : "current",
               static_cast<unsigned>(SMP_ENABLED), sample,
               static_cast<double>(now() - start) / iterations);
    }
    if (switches != 2 * calls || caller.core.quantum != 7 || server.core.quantum != 7
        || caller.core.source_reply_target || server.core.destination_reply_target
        || caller.core.status != process_status::READY
        || server.core.status != process_status::BLOCKED_RECEIVE
        || checksum != samples * iterations * (iterations + 1) / 2)
    {
        abort();
    }
}
