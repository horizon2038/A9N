#include <gtest/gtest.h>

#include <kernel/capability/ipc_port.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/process/process.hpp>

TEST(ipc_port_test, ipc_send_test)
{
    a9n::kernel::capability_slot slot;
    a9n::kernel::ipc_port        ipc_port;

    // sender configuration
    a9n::kernel::v2::process     sender_1;
    a9n::kernel::ipc_buffer      sender_buffer_1;
    a9n::kernel::capability_slot sender_slot_1;
    sender_1.buffer = &sender_buffer_1;
    sender_buffer_1.message_tag
        = static_cast<a9n::word>(a9n::kernel::ipc_port_operation::SEND);
    sender_buffer_1.set_message<
        static_cast<a9n::word>(a9n::kernel::ipc_port_send_argument::MESSAGE_COUNT)>(1
    );
    sender_buffer_1.set_message<
        static_cast<a9n::word>(a9n::kernel::ipc_port_send_argument::IS_BLOCK)>(1);

    // receiver configuration
    a9n::kernel::v2::process     receiver_1;
    a9n::kernel::ipc_buffer      receiver_buffer_1;
    a9n::kernel::capability_slot receiver_slot_1;
    receiver_1.buffer = &receiver_buffer_1;
}
