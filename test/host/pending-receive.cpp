// Use the real IPC/notification implementations, substituting only scheduling
// and HAL message-register access. No SDK or IRQ timing is involved.
#include <kernel/capability/ipc_port.hpp>
#include <kernel/capability/notification_port.hpp>
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
    unsigned switches {};
    unsigned direct_switches {};
    process *expected_direct_target {};
    cpu_local_variable *expected_direct_local {};
    a9n::word failed_register = 64;

    struct test_process
    {
        process core {};
        ipc_buffer buffer {};
        test_process()
        {
            core.status = process_status::READY;
            core.buffer = &buffer;
        }
    };

    struct test_endpoint
    {
        ipc_port port {};
        capability_slot slot {};
        test_endpoint() { slot.rights = capability_slot::ALL; }
        capability_result invoke(test_process &target, a9n::word operation, bool block = true,
                                 a9n::word length = 0)
        {
            target.buffer.messages[1] = operation;
            target.buffer.messages[2] = (length << 1) | static_cast<a9n::word>(block);
            target.buffer.messages[3] = 0;
            return port.execute(target.core, slot);
        }
    };

    struct test_notification
    {
        notification_port port {};
        capability_slot slot {};
        explicit test_notification(test_process &target)
        {
            CHECK(try_configure_notification_port_slot(target.core.binded_notification_port, port, 1));
            CHECK(try_configure_notification_port_slot(slot, port, 1));
            CHECK(port.bind_process(target.core));
        }
        void notify(test_process &source) { CHECK(port.operation_notify(source.core, slot)); }
    };

    bool delivered(const test_process &target)
    {
        return ((target.buffer.messages[2] >> 13) & 3) == 2
            && target.buffer.messages[3] == 1;
    }
}

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> get_message_register(const process &target, a9n::word index)
    {
        CHECK(index < 64);
        CHECK(target.buffer);
        return target.buffer->messages[index];
    }
    hal_result configure_message_register(process &target, a9n::word index, a9n::word value)
    {
        CHECK(index < 64);
        if (index == failed_register)
        {
            return hal_error::TRY_AGAIN;
        }
        CHECK(target.buffer);
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
    kernel_result try_schedule_and_switch(process &) { ++switches; return {}; }
    kernel_result mark_scheduled(process &, process &target)
    {
        target.status = process_status::READY;
        return {};
    }
    kernel_result mark_scheduled_with_preemption(process &owner, process &target)
    {
        return mark_scheduled(owner, target);
    }
    kernel_result try_direct_schedule_and_switch(process &, process &target)
    {
        CHECK(&target == expected_direct_target && !expected_direct_local);
        ++direct_switches;
        return {};
    }
    kernel_result process_manager::try_direct_schedule_and_switch(
        process &target, cpu_local_variable &local)
    {
        CHECK(&target == expected_direct_target && &local == expected_direct_local);
        ++direct_switches;
        return {};
    }
}
namespace a9n::kernel::utility
{
    void logger::printk(const char *, ...) {}
    void logger::printh(const char *, ...) {}
    void logger::error(const char *) { abort(); }
}

static void pending_before_receive(a9n::word operation, bool block)
{
    test_process server, sender;
    test_endpoint endpoint;
    test_notification notification(server);
    notification.notify(sender);
    CHECK(notification.port.has_pending_notification());
    switches = 0;
    CHECK(endpoint.invoke(server, operation, block));
    printf("%s block=%u: pending=%u blocked=%u switches=%u notification=%u\n",
           operation == 5 ? "Reply-Receive" : "Receive", static_cast<unsigned>(block),
           static_cast<unsigned>(notification.port.has_pending_notification()),
           static_cast<unsigned>(server.core.status == process_status::BLOCKED_RECEIVE),
           switches, static_cast<unsigned>(delivered(server)));
    fflush(stdout);
    CHECK(server.core.status == process_status::READY);
    CHECK(!server.core.current_ipc_port && switches == 0);
    CHECK(!notification.port.has_pending_notification() && delivered(server));

    // No phantom READY_TO_RECEIVE state: there is no recipient for this send.
    CHECK(endpoint.invoke(sender, 1, false));
    CHECK(sender.core.status == process_status::READY);
    // Notification is consumed only once; the next receive really does wait.
    CHECK(endpoint.invoke(server, 2));
    CHECK(server.core.status == process_status::BLOCKED_RECEIVE && switches == 1);
    notification.notify(sender);
    CHECK(server.core.status == process_status::READY && delivered(server));
    CHECK(!server.core.current_ipc_port);
}

static void existing_receiver_is_preserved()
{
    test_process first, second, sender;
    test_endpoint endpoint;
    test_notification notification(second);
    CHECK(endpoint.invoke(first, 2));
    notification.notify(sender);
    CHECK(endpoint.invoke(second, 2));
    CHECK(delivered(second) && !second.core.current_ipc_port);
    CHECK(first.core.status == process_status::BLOCKED_RECEIVE);
    sender.buffer.messages[4] = 0x1234;
    CHECK(endpoint.invoke(sender, 1, true, 1));
    CHECK(first.core.status == process_status::READY && !first.core.current_ipc_port);
    CHECK(first.buffer.messages[4] == 0x1234);
    CHECK(delivered(second));
}

static void ready_sender_keeps_precedence()
{
    test_process server, sender;
    test_endpoint endpoint;
    test_notification notification(server);
    sender.buffer.messages[4] = 0x5678;
    CHECK(endpoint.invoke(sender, 1, true, 1));
    CHECK(sender.core.status == process_status::BLOCKED_SEND);
    notification.notify(server);
    CHECK(endpoint.invoke(server, 2));
    CHECK(server.core.status == process_status::READY && sender.core.status == process_status::READY);
    CHECK(!delivered(server) && server.buffer.messages[4] == 0x5678);
    CHECK(notification.port.has_pending_notification());
    CHECK(endpoint.invoke(server, 2));
    CHECK(delivered(server) && !notification.port.has_pending_notification());
}

static void empty_nonblocking_receive_keeps_endpoint_idle()
{
    test_process server, sender;
    test_endpoint endpoint;
    switches = 0;
    CHECK(endpoint.invoke(server, 2, false));
    CHECK(server.core.status == process_status::READY && !server.core.current_ipc_port);
    CHECK(endpoint.invoke(sender, 1, false));
    CHECK(sender.core.status == process_status::READY && switches == 0);
}

static void receive_discards_old_reply_before_delivering_notification()
{
    test_process server, caller;
    test_endpoint endpoint;
    test_notification notification(server);
    server.core.destination_reply_state = process::destination_reply_state_object::READY_TO_REPLY;
    server.core.destination_reply_target = &caller.core;
    caller.core.source_reply_state = process::source_reply_state_object::WAIT;
    caller.core.source_reply_target = &server.core;
    caller.core.status = process_status::BLOCKED_REPLY;
    notification.notify(server);
    CHECK(endpoint.invoke(server, 2));
    CHECK(delivered(server));
    CHECK(!server.core.destination_reply_target && !caller.core.source_reply_target);
    CHECK(server.core.destination_reply_state == process::destination_reply_state_object::NONE);
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::NONE);
    CHECK(caller.core.status == process_status::READY);
    CHECK(caller.buffer.messages[0] == 0);
    CHECK(caller.buffer.messages[1] == static_cast<a9n::word>(capability_error::ILLEGAL_OPERATION));
}

static void errors_do_not_enqueue_receiver()
{
    const a9n::word register_indices[] { 2, 3 };
    for (auto index : register_indices)
    {
        test_process server, sender;
        test_endpoint endpoint;
        test_notification notification(server);
        notification.notify(sender);
        failed_register = index;
        CHECK(!endpoint.invoke(server, 2));
        failed_register = 64;
        CHECK(server.core.status == process_status::READY && !server.core.current_ipc_port);
        CHECK(endpoint.invoke(sender, 1, false));
    }
    test_process server;
    test_endpoint endpoint;
    test_notification notification(server);
    notification.notify(server);
    endpoint.slot.rights = capability_slot::WRITE;
    CHECK(!endpoint.invoke(server, 2));
    CHECK(notification.port.has_pending_notification());
    CHECK(server.core.status == process_status::READY && !server.core.current_ipc_port);
}

static void fast_reply_receive_preserves_quantum(a9n::sword remaining, a9n::word core)
{
    test_process server, caller;
    test_endpoint endpoint;
    server.core.core_affinity = caller.core.core_affinity = core;
    server.core.quantum = 5;
    caller.core.quantum = remaining;
    switches = direct_switches = 0;
    CHECK(endpoint.invoke(server, 2));
    CHECK(server.core.status == process_status::BLOCKED_RECEIVE && switches == 1);

    for (unsigned call = 0; call < 1000; ++call)
    {
        expected_direct_target = &server.core;
        expected_direct_local = nullptr;
        caller.buffer.messages[4] = call;
        CHECK(endpoint.invoke(caller, 3, true, 1));
        CHECK(caller.core.status == process_status::BLOCKED_REPLY);
        CHECK(server.core.status == process_status::READY && !server.core.current_ipc_port);
        CHECK(server.core.destination_reply_target == &caller.core);
        CHECK(server.buffer.messages[4] == call);

        expected_direct_target = &caller.core;
        expected_direct_local = &cpu_local_variables[SMP_ENABLED ? core : BSP_ID];
        server.buffer.messages[4] = call + 0x1000;
        CHECK(endpoint.invoke(server, 5, true, 1));
        CHECK(caller.core.quantum == remaining && server.core.quantum == 5);
        CHECK(caller.core.status == process_status::READY);
        CHECK(server.core.status == process_status::BLOCKED_RECEIVE);
        CHECK(server.core.current_ipc_port == &endpoint.port);
        CHECK(!caller.core.source_reply_target && !server.core.destination_reply_target);
        CHECK(caller.core.source_reply_state == process::source_reply_state_object::NONE);
        CHECK(server.core.destination_reply_state == process::destination_reply_state_object::NONE);
        CHECK(caller.buffer.messages[4] == call + 0x1000);
        CHECK(direct_switches == 2 * (call + 1) && switches == 1);
    }
    expected_direct_target = nullptr;
    expected_direct_local = nullptr;
}

static void revoke_cancels_only_queued_calls()
{
    test_process server, caller, queued;
    test_endpoint endpoint, reply_endpoint;
    // Queue-first acceptance exercises the slow CALL path's reply back-link.
    caller.buffer.messages[0] = 0x1357;
    CHECK(endpoint.invoke(caller, 3));
    CHECK(endpoint.invoke(server, 2));
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::WAIT);
    CHECK(caller.core.source_reply_target == &server.core);
    CHECK(endpoint.invoke(queued, 3));
    CHECK(endpoint.port.revoke(endpoint.slot));
    CHECK(caller.core.status == process_status::BLOCKED_REPLY);
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::WAIT);
    CHECK(caller.core.source_reply_target == &server.core);
    CHECK(server.core.destination_reply_target == &caller.core);
    CHECK(server.core.destination_reply_state == process::destination_reply_state_object::READY_TO_REPLY);
    CHECK(caller.buffer.messages[0] == 0x1357 && caller.buffer.messages[1] == 3);
    CHECK(queued.core.status == process_status::READY && !queued.core.current_ipc_port);
    CHECK(queued.core.source_reply_state == process::source_reply_state_object::NONE);
    CHECK(queued.buffer.messages[0] == 0);
    CHECK(queued.buffer.messages[1] == static_cast<a9n::word>(capability_error::ILLEGAL_OPERATION));
    CHECK(endpoint.port.revoke(endpoint.slot)); // idempotent after cancellation

    // Reply uses the receiver's saved relationship, not the original Port.
    server.buffer.messages[4] = 0x2468;
    expected_direct_target = &server.core;
    CHECK(reply_endpoint.invoke(server, 4, true, 1));
    expected_direct_target = nullptr;
    CHECK(caller.core.status == process_status::READY);
    CHECK(caller.buffer.messages[4] == 0x2468);
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::NONE);
    CHECK(!caller.core.source_reply_target && !server.core.destination_reply_target);

    test_notification notification(server);
    CHECK(notification.port.operation_wait(server.core, notification.slot));
    CHECK(server.core.status == process_status::BLOCKED_WAIT);
    CHECK(notification.port.revoke(notification.slot));
    CHECK(server.core.status == process_status::READY);
    CHECK(!server.core.current_notification_port);
    CHECK(!server.core.binded_notification_port.component);
}

static void reply_survives_port_destruction_and_reuse()
{
    test_process server, caller, other_server, other_caller;
    test_endpoint endpoint, reply_endpoint;
    CHECK(endpoint.invoke(server, 2));
    expected_direct_target = &server.core;
    CHECK(endpoint.invoke(caller, 3));
    expected_direct_target = nullptr;
    CHECK(endpoint.port.revoke(endpoint.slot));

    // Reuse the exact address: the old caller must retain no Port reference.
    endpoint.port.~ipc_port();
    new (&endpoint.port) ipc_port {};
    CHECK(endpoint.invoke(other_server, 2));
    expected_direct_target = &other_server.core;
    CHECK(endpoint.invoke(other_caller, 3));
    expected_direct_target = nullptr;
    CHECK(endpoint.port.revoke(endpoint.slot));
    CHECK(caller.core.source_reply_target == &server.core);
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::WAIT);
    CHECK(caller.core.status == process_status::BLOCKED_REPLY);
    CHECK(other_caller.core.source_reply_target == &other_server.core);
    CHECK(other_caller.core.source_reply_state == process::source_reply_state_object::WAIT);
    CHECK(other_caller.core.status == process_status::BLOCKED_REPLY);

    server.buffer.messages[4] = 0x1234;
    other_server.buffer.messages[4] = 0x5678;
    expected_direct_target = &server.core;
    CHECK(reply_endpoint.invoke(server, 4, true, 1));
    CHECK(caller.core.status == process_status::READY && caller.buffer.messages[4] == 0x1234);
    CHECK(other_caller.core.status == process_status::BLOCKED_REPLY);
    CHECK(other_server.core.destination_reply_target == &other_caller.core);
    expected_direct_target = &other_server.core;
    CHECK(reply_endpoint.invoke(other_server, 4, true, 1));
    expected_direct_target = nullptr;
    CHECK(other_caller.core.status == process_status::READY);
    CHECK(other_caller.buffer.messages[4] == 0x5678);
}

static void revoke_cancels_queued_receivers()
{
    test_process server;
    test_endpoint endpoint;
    CHECK(endpoint.invoke(server, 2));
    CHECK(server.core.status == process_status::BLOCKED_RECEIVE);
    CHECK(endpoint.port.revoke(endpoint.slot));
    CHECK(server.core.status == process_status::READY && !server.core.current_ipc_port);
    CHECK(server.buffer.messages[0] == 0);
    CHECK(server.buffer.messages[1] == static_cast<a9n::word>(capability_error::ILLEGAL_OPERATION));
}

static void pcb_reply_cleanup_remains_independent()
{
    test_process server, caller;
    test_endpoint endpoint;
    CHECK(endpoint.invoke(server, 2));
    expected_direct_target = &server.core;
    CHECK(endpoint.invoke(caller, 3));
    expected_direct_target = nullptr;
    CHECK(endpoint.port.revoke(endpoint.slot));

    // A destroyed/suspended caller must no longer be a server's reply target.
    caller.core.status = process_status::BLOCKED_SUSPEND;
    caller.buffer.messages[0] = 0x1234;
    caller.buffer.messages[1] = 0x5678;
    CHECK(ipc_port::cancel_reply(caller.core, ipc_port::reply_cancellation_mode::DETACH_ONLY));
    CHECK(caller.core.status == process_status::BLOCKED_SUSPEND);
    CHECK(caller.buffer.messages[0] == 0x1234 && caller.buffer.messages[1] == 0x5678);
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::NONE);
    CHECK(!caller.core.source_reply_target && !server.core.destination_reply_target);
    CHECK(server.core.destination_reply_state == process::destination_reply_state_object::NONE);

    CHECK(endpoint.invoke(server, 2));
    caller.core.status = process_status::READY;
    expected_direct_target = &server.core;
    CHECK(endpoint.invoke(caller, 3));
    expected_direct_target = nullptr;
    // PCB revoke uses this path when destroying the receiver of a CALL.
    CHECK(ipc_port::cancel_reply(caller.core, ipc_port::reply_cancellation_mode::RESUME_WITH_ERROR));
    CHECK(caller.core.status == process_status::READY);
    CHECK(caller.core.source_reply_state == process::source_reply_state_object::NONE);
    CHECK(!caller.core.source_reply_target && !server.core.destination_reply_target);
    CHECK(caller.buffer.messages[0] == 0);
    CHECK(caller.buffer.messages[1] == static_cast<a9n::word>(capability_error::ILLEGAL_OPERATION));
}

int main()
{
    // Use the wire operation numbers through execute(), not private helpers.
    pending_before_receive(5, true);
    pending_before_receive(2, true);
    pending_before_receive(2, false);
    existing_receiver_is_preserved();
    ready_sender_keeps_precedence();
    empty_nonblocking_receive_keeps_endpoint_idle();
    receive_discards_old_reply_before_delivering_notification();
    errors_do_not_enqueue_receiver();
    fast_reply_receive_preserves_quantum(3, BSP_ID);
    fast_reply_receive_preserves_quantum(0, BSP_ID);
    fast_reply_receive_preserves_quantum(7, SMP_ENABLED ? 2 : BSP_ID);
    revoke_cancels_only_queued_calls();
    reply_survives_port_destruction_and_reuse();
    revoke_cancels_queued_receivers();
    pcb_reply_cleanup_remains_independent();
    puts("PASS: Receive/Reply-Receive, quantum, queued cancellation, independent replies and Port reuse");
}
