// Use the real IPC/notification implementations, substituting only scheduling
// and HAL message-register access. No SDK or IRQ timing is involved.
#include <kernel/capability/ipc_port.hpp>
#include <kernel/capability/notification_port.hpp>
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
        if (index == failed_register) return hal_error::TRY_AGAIN;
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
    template<bool Quantum> kernel_result try_direct_schedule_and_switch(process &, process &)
    {
        abort();
    }
    template kernel_result try_direct_schedule_and_switch<true>(process &, process &);
    template kernel_result try_direct_schedule_and_switch<false>(process &, process &);
    template<bool Quantum> kernel_result process_manager::try_direct_schedule_and_switch(
        process &, cpu_local_variable &)
    {
        abort();
    }
    template kernel_result process_manager::try_direct_schedule_and_switch<false>(process &, cpu_local_variable &);
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
    CHECK(caller.core.status == process_status::BLOCKED_REPLY); // Dropped, not replied to.
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
    puts("PASS: 10 pending Receive, queue-state and error scenarios");
}
