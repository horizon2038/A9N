#include <kernel/capability/process_control_block.hpp>
#include <kernel/capability/ipc_port.hpp>
#include <kernel/capability/notification_port.hpp>
#include <kernel/utility/logger.hpp>
#include <hal/interface/cpu.hpp>
#include <stdio.h>
#include <stdlib.h>

using namespace a9n::kernel;
#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); abort(); \
} } while (0)

namespace
{
    a9n::word messages[64] {};
    a9n::word failed_read = ~a9n::word { 0 };
    int failed_register = -1;
    int failed_finalizer = -1;
    bool failed_stop = false;
    unsigned stop_calls = 0;
    process *stopped_process = nullptr;
    unsigned ipc_detaches = 0;
    unsigned notification_detaches = 0;
    unsigned restarted_syscalls = 0;
    unsigned scheduled = 0;
    a9n::word failed_result_register = ~a9n::word { 0 };
    bool failed_detach = false;
    a9n::word checksum = 1469598103934665603;
    unsigned cases = 0;
    void record(a9n::word value) { checksum = (checksum ^ value) * 1099511628211; }

    class source_node : public capability_component
    {
      public:
        capability_slot slots[5] {};
        capability_result execute(process &, capability_slot &) override { abort(); }
        capability_result revoke(capability_slot &self) override
        {
            CHECK(stopped_process && stopped_process->status == process_status::BLOCKED_SUSPEND);
            record(500 + self.data[0]);
            if (static_cast<int>(self.data[0]) == failed_finalizer)
            {
                return capability_error::FATAL;
            }
            return {};
        }
        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            if (index >= 5)
            {
                return capability_lookup_error::INDEX_OUT_OF_RANGE;
            }
            return &slots[index];
        }
        capability_lookup_result traverse_slot(a9n::word descriptor, a9n::word, a9n::word) override
        {
            record(100 + descriptor);
            return retrieve_slot(descriptor);
        }
    };
}

namespace a9n::hal
{
    a9n::word core_count() { return 2; }
    liba9n::result<a9n::word, hal_error> current_core_number() { return a9n::word { 0 }; }
    liba9n::result<a9n::word, hal_error> get_message_register(const process &, a9n::word index)
    {
        CHECK(index < 64);
        record(index);
        if (index == failed_read)
        {
            return hal_error::TRY_AGAIN;
        }
        return messages[index];
    }
    hal_result init_hardware_context(cpu_mode, hardware_context &) { record(200); return {}; }
    hal_result configure_floating_context(floating_context &) { record(201); return {}; }
    bool is_valid_user_address(a9n::virtual_address value) { return value < (a9n::word { 1 } << 47); }
    hal_result configure_general_register(process &, register_type type, a9n::word value)
    {
        record(300 + static_cast<unsigned>(type));
        record(value);
        if (static_cast<int>(type) == failed_register)
        {
            return hal_error::TRY_AGAIN;
        }
        return {};
    }
    hal_result configure_message_register(process &target, a9n::word index, a9n::word value)
    {
        if (index == failed_result_register)
        {
            return hal_error::TRY_AGAIN;
        }
        CHECK(index < 2);
        target.registers[4 + index] = value; // x86-64 MR0=RDI, MR1=RSI
        return {};
    }
    void restart_syscall(process &target)
    {
        CHECK(target.status == process_status::BLOCKED_SUSPEND);
        target.registers[15] -= 2;
        ++restarted_syscalls;
    }
}

namespace a9n::kernel
{
    kernel_result mark_scheduled(process &, process &target)
    {
        ++scheduled;
        target.status = process_status::READY;
        return {};
    }
    kernel_result reschedule_core(a9n::word) { abort(); }
    kernel_result process_manager::mark_suspended(process &) { abort(); }
    kernel_result process_manager::try_schedule_and_switch() { abort(); }
    kernel_result stop_process_for_reclamation(process &target)
    {
        ++stop_calls;
        if (failed_stop)
        {
            return kernel_error::UNEXPECTED;
        }
        target.status = process_status::BLOCKED_SUSPEND;
        stopped_process = &target;
        return {};
    }
    capability_result ipc_port::cancel_reply(process &target, reply_cancellation_mode mode)
    {
        CHECK(mode == reply_cancellation_mode::DETACH_ONLY);
        auto *server = target.source_reply_target;
        if (server && server->destination_reply_target == &target)
        {
            server->destination_reply_target = nullptr;
            server->destination_reply_state = process::destination_reply_state_object::NONE;
        }
        target.source_reply_target = nullptr;
        target.source_reply_state = process::source_reply_state_object::NONE;
        return {};
    }
    kernel_result ipc_port::remove_ipc_queue(process &target)
    {
        ++ipc_detaches;
        if (failed_detach)
        {
            return kernel_error::UNEXPECTED;
        }
        target.current_ipc_port = nullptr;
        return {};
    }
    kernel_result notification_port::remove_notification_queue(process &target)
    {
        ++notification_detaches;
        if (failed_detach)
        {
            return kernel_error::UNEXPECTED;
        }
        target.current_notification_port = nullptr;
        return {};
    }
    kernel_result notification_port::bind_process(process &) { record(400); return {}; }
    kernel_result notification_port::unbind_process(process &) { record(401); return {}; }
    capability_result notification_port::execute(process &, capability_slot &) { abort(); }
    capability_result notification_port::revoke(capability_slot &self)
    {
        CHECK(stopped_process && stopped_process->status == process_status::BLOCKED_SUSPEND);
        record(500 + self.data[0]);
        if (failed_finalizer == 4)
        {
            return capability_error::FATAL;
        }
        return {};
    }
}

namespace a9n::kernel::utility
{
    void logger::error(const char *) { abort(); }
}

int main()
{
    source_node source;
    notification_port notification;
    const capability_type types[] {
        capability_type::ADDRESS_SPACE, capability_type::NODE, capability_type::FRAME,
        capability_type::NOTIFICATION_PORT, capability_type::IPC_PORT,
    };
    for (unsigned index = 0; index < 5; ++index)
    {
        auto &slot = source.slots[index];
        slot.init();
        slot.type = types[index];
        slot.component = &source;
        slot.rights = capability_slot::ALL;
    }
    source.slots[3].component = &notification;
    process owner {};
    owner.root_slot.component = &source;
    for (unsigned profile = 0; profile < 21; ++profile)
    {
        for (a9n::word flags = 0; flags < 1024; ++flags)
        {
            process_control_block pcb;
            capability_slot self {};
            CHECK(try_configure_process_control_block_slot(self, pcb));
            messages[2] = flags;
            for (unsigned index = 0; index < 5; ++index)
            {
                messages[index + 3] = index;
            }
            messages[8] = 0x1000;
            messages[9] = 0x2000;
            messages[10] = 0x3000;
            messages[11] = 7;
            messages[12] = 1;
            failed_read = ~a9n::word { 0 };
            failed_register = -1;
            if (profile >= 1 && profile <= 11)
            {
                failed_read = profile + 1;
            }
            if (profile >= 12 && profile <= 14)
            {
                messages[profile - 4] = a9n::word { 1 } << 63;
            }
            if (profile == 15)
            {
                messages[11] = PRIORITY_MAX;
            }
            if (profile == 16)
            {
                messages[12] = 2;
            }
            if (profile >= 17 && profile <= 19)
            {
                failed_register = profile - 17;
            }
            if (profile == 20)
            {
                pcb.process_core.status = process_status::READY;
            }
            record(profile);
            record(flags);
            auto configured = pcb.operation_configure(owner, self);
            record(static_cast<bool>(configured));
            if (!configured)
            {
                record(static_cast<a9n::word>(configured.unwrap_error()));
            }
            if (profile == 0 || (flags == 0 && failed_read != 2))
            {
                CHECK(configured);
            }
            record(pcb.process_core.priority);
            record(pcb.process_core.core_affinity);
            record(pcb.process_core.buffer != nullptr);
            stopped_process = nullptr;
            stop_calls = 0;
            CHECK(pcb.revoke(self));
            CHECK(stop_calls == 1 && stopped_process == &pcb.process_core);
            for (const auto &slot : source.slots)
            {
                CHECK(!slot.has_sibling());
            }
            ++cases;
        }
    }
    for (int failure = 0; failure < 5; ++failure)
    {
        process_control_block pcb;
        source_node objects[4];
        capability_slot self {};
        CHECK(try_configure_process_control_block_slot(self, pcb));
        capability_slot *slots[] {
            &pcb.process_core.root_slot, &pcb.process_core.root_address_space,
            &pcb.process_core.buffer_frame, &pcb.process_core.resolver_port,
            &pcb.process_core.binded_notification_port,
        };
        for (unsigned index = 0; index < 5; ++index)
        {
            slots[index]->init();
            slots[index]->rights = capability_slot::ALL;
            slots[index]->type = capability_type::FRAME;
            slots[index]->data[0] = index;
            if (index < 4)
            {
                slots[index]->component = &objects[index];
            }
            else
            {
                slots[index]->component = &notification;
                slots[index]->type = capability_type::NOTIFICATION_PORT;
            }
        }
        auto *sentinel = reinterpret_cast<ipc_buffer *>(0x1000);
        pcb.process_core.buffer = sentinel;
        stopped_process = nullptr;
        stop_calls = 0;
        failed_finalizer = failure;
        CHECK(!pcb.revoke(self));
        CHECK(stop_calls == 1 && stopped_process == &pcb.process_core);
        CHECK(slots[failure]->component != nullptr);
        if (failure < 2)
        {
            CHECK(pcb.process_core.buffer == sentinel);
        }
        else
        {
            CHECK(pcb.process_core.buffer == nullptr);
        }
        failed_finalizer = -1;
        CHECK(pcb.revoke(self));
        CHECK(stop_calls == 2);
        ++cases;
    }
    {
        process_control_block pcb;
        source_node resource, allocation;
        capability_slot self {}, parent {};
        CHECK(try_configure_process_control_block_slot(self, pcb));
        parent.type = capability_type::GENERIC;
        parent.component = &allocation;
        parent.data[0] = 4;
        parent.insert_child(self);
        // Exercise the complete Node -> Object path, not a separate prepare
        // call. A failed stop must leave the slot and owned resources intact.
        pcb.process_core.root_slot.type = capability_type::NODE;
        pcb.process_core.root_slot.component = &resource;
        pcb.process_core.status = process_status::READY;
        auto saved_checksum = checksum;
        stopped_process = nullptr;
        stop_calls = 0;
        failed_stop = true;
        CHECK(!capability_node::revoke_slot(parent));
        CHECK(stop_calls == 1 && stopped_process == nullptr);
        CHECK(checksum == saved_checksum);
        CHECK(self.component == &pcb && !self.reclaiming);
        CHECK(parent.component == &allocation && parent.has_child() && !parent.reclaiming);
        CHECK(pcb.process_core.root_slot.component == &resource);
        failed_stop = false;
        CHECK(capability_node::revoke_slot(parent));
        CHECK(stop_calls == 2 && stopped_process == &pcb.process_core);
        CHECK(self.type == capability_type::NONE);
        CHECK(parent.component == &allocation && !parent.has_child());
        CHECK(pcb.process_core.root_slot.type == capability_type::NONE);
        ++cases;
    }
    // Pin the pre-readability-refactor order of argument reads, mutations,
    // errors and teardown, independently of the production flag/table names.
    constexpr unsigned expected_cases = 21510;
    constexpr a9n::word expected_trace = 12865827607116404806ULL;
    CHECK(cases == expected_cases);
    CHECK(checksum == expected_trace);
    printf("PCB configuration/reclamation: cases=%u trace=%llu PASS\n", cases, static_cast<unsigned long long>(checksum));
    // SUSPEND must reach the same synchronous stop routine as revoke, including
    // for a running remote target. The helper's ACK protocol is tested separately.
    {
        process_control_block pcb;
        capability_slot self {};
        CHECK(try_configure_process_control_block_slot(self, pcb));
        auto &target = pcb.process_core;
        target.core_affinity = 1;
        target.priority = 17;
        target.status = process_status::READY;
        auto &remote = cpu_local_variables[1];
        remote.current_process = &target;
        remote.is_idle = false;
        stopped_process = nullptr;
        stop_calls = 0;
        failed_stop = true;
        CHECK(!pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 1 && stopped_process == nullptr);
        CHECK(target.status == process_status::READY && target.priority == 17);
        failed_stop = false;
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 2 && stopped_process == &target);
        CHECK(target.status == process_status::BLOCKED_SUSPEND && target.priority == 17);
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 3);
        remote.current_process = nullptr;
        remote.is_idle = true;

        target.current_ipc_port = reinterpret_cast<ipc_port *>(0x1000);
        failed_detach = true;
        CHECK(!pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 4 && ipc_detaches == 1);
        CHECK(target.current_ipc_port != nullptr);
        failed_detach = false;
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 5 && ipc_detaches == 2 && target.current_ipc_port == nullptr);

        target.current_notification_port = &notification;
        failed_detach = true;
        CHECK(!pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 6 && notification_detaches == 1);
        CHECK(target.current_notification_port == &notification);
        failed_detach = false;
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(stop_calls == 7 && notification_detaches == 2 && target.current_notification_port == nullptr);

        // The caller may suspend itself; it must use the same stop routine.
        CHECK(pcb.operation_suspend(target, self));
        CHECK(stop_calls == 8 && stopped_process == &target);
    }
    puts("PCB synchronous suspend: routing, stop failure/retry, idempotence, queue detach and self suspend PASS");

    const process_status waits[] {
        process_status::BLOCKED_SEND, process_status::BLOCKED_RECEIVE,
        process_status::BLOCKED_REPLY, process_status::BLOCKED_WAIT,
    };
    for (auto status : waits)
    {
        process_control_block pcb;
        capability_slot self {};
        auto &target = pcb.process_core;
        target.status = status;
        target.registers[15] = 0x4002;
        target.registers[4] = 0x1234;
        target.registers[5] = 0x5678;
        const auto before = scheduled;
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(scheduled == before && target.status == process_status::BLOCKED_SUSPEND);
        CHECK(target.registers[4] == 0);
        CHECK(target.registers[5] == static_cast<a9n::word>(capability_error::ILLEGAL_OPERATION));
        CHECK(target.registers[15] == 0x4002);
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(target.registers[15] == 0x4002 && scheduled == before);
    }
    // Fault cancellation preserves every syscall argument, even when a
    // detach fails and the same Suspend must be retried.
    const fault_type faults[] {
        fault_type::INVALID_KERNEL_CALL, fault_type::MEMORY,
        fault_type::INVALID_INSTRUCTION, fault_type::INVALID_ARITHMETIC,
    };
    for (auto fault : faults)
    {
        process_control_block pcb;
        capability_slot self {};
        process server {}, caller {};
        auto &target = pcb.process_core;
        target.status = process_status::BLOCKED_FAULT;
        target.fault_reason = fault;
        target.fault_code = 99;
        target.arch_fault_code = 42;
        target.fault_address = 0x4002;
        target.current_ipc_port = reinterpret_cast<ipc_port *>(0x1000);
        target.source_reply_state = process::source_reply_state_object::WAIT;
        target.source_reply_target = &server;
        server.destination_reply_target = &target;
        server.destination_reply_state = process::destination_reply_state_object::READY_TO_REPLY;
        target.destination_reply_target = &caller;
        target.destination_reply_state = process::destination_reply_state_object::READY_TO_REPLY;
        for (a9n::word index = 0; index < target.registers.size(); ++index)
        {
            target.registers[index] = 0x4000 + index;
        }
        auto original = target.registers;
        const auto before = restarted_syscalls;
        failed_detach = true;
        CHECK(!pcb.operation_suspend(owner, self));
        CHECK(target.fault_reason == fault && restarted_syscalls == before);
        CHECK(target.registers[15] == original[15]);
        failed_detach = false;
        CHECK(pcb.operation_suspend(owner, self));
        auto expected_pc = original[15];
        if (fault == fault_type::INVALID_KERNEL_CALL)
        {
            expected_pc -= 2;
        }
        for (a9n::word index = 0; index < target.registers.size(); ++index)
        {
            const auto expected = index == 15 ? expected_pc : original[index];
            CHECK(target.registers[index] == expected);
        }
        CHECK(target.fault_reason == fault_type::NONE && target.fault_code == 0);
        CHECK(target.arch_fault_code == 0 && target.fault_address == 0);
        CHECK(!target.current_ipc_port && !target.source_reply_target);
        CHECK(target.source_reply_state == process::source_reply_state_object::NONE);
        CHECK(!server.destination_reply_target);
        CHECK(target.destination_reply_target == &caller);
        CHECK(target.destination_reply_state == process::destination_reply_state_object::READY_TO_REPLY);
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(target.registers[15] == expected_pc);
        // A replacement signal handler PC must survive Resume untouched.
        target.registers[15] = 0x8000;
        CHECK(pcb.operation_resume(owner, self));
        CHECK(target.status == process_status::READY && target.registers[15] == 0x8000);
    }
    for (a9n::word failed_index = 0; failed_index < 2; ++failed_index)
    {
        process_control_block pcb;
        capability_slot self {};
        auto &target = pcb.process_core;
        target.status = process_status::BLOCKED_SEND;
        target.current_ipc_port = reinterpret_cast<ipc_port *>(0x1000);
        failed_result_register = failed_index;
        CHECK(!pcb.operation_suspend(owner, self));
        CHECK(target.current_ipc_port && target.status == process_status::BLOCKED_SUSPEND);
        failed_result_register = ~a9n::word { 0 };
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(!target.current_ipc_port && target.registers[4] == 0);
        CHECK(target.registers[5] == static_cast<a9n::word>(capability_error::ILLEGAL_OPERATION));
    }
    {
        // Direct Resume is also used by parked fault handlers. It must not
        // replay a syscall whose result/context they have already installed.
        process_control_block pcb;
        capability_slot self {};
        process server {};
        auto &target = pcb.process_core;
        target.status = process_status::BLOCKED_REPLY;
        target.fault_reason = fault_type::INVALID_KERNEL_CALL;
        target.fault_code = 99;
        target.source_reply_target = &server;
        target.source_reply_state = process::source_reply_state_object::WAIT;
        server.destination_reply_target = &target;
        target.registers[15] = 0x4002;
        target.registers[0] = 123;
        const auto before = restarted_syscalls;
        CHECK(pcb.operation_resume(owner, self));
        CHECK(restarted_syscalls == before && target.registers[15] == 0x4002);
        CHECK(target.registers[0] == 123 && target.fault_reason == fault_type::NONE);
        CHECK(!server.destination_reply_target && !target.source_reply_target);
        CHECK(pcb.operation_suspend(owner, self));
        CHECK(target.registers[15] == 0x4002 && restarted_syscalls == before);
    }
    puts("PCB suspend continuation: native cancellation, fault argument preservation, retries, incoming replies and explicit Resume context PASS");
}
