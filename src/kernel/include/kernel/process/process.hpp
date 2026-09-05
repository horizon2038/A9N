#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <hal/arch/arch_types.hpp>
#include <kernel/interrupt/fault.hpp>
#include <kernel/types.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/ipc_buffer.hpp>

#include <stdint.h>

namespace a9n::kernel
{
    inline constexpr a9n::word QUANTUM_MAX      = 10;
    inline constexpr a9n::word STACK_SIZE_MAX   = 8192;
    inline constexpr a9n::word PROCESS_NAME_MAX = 128;

    using hardware_context = liba9n::std::array<a9n::word, a9n::hal::HARDWARE_CONTEXT_SIZE>;
    using floating_context = liba9n::std::array<a9n::word, a9n::hal::FLOATING_CONTEXT_SIZE>;

    enum class process_status : uint16_t
    {
        UNUSED,
        READY,
        // for IPC / Notification
        BLOCKED_SEND,
        BLOCKED_RECEIVE,
        BLOCKED_REPLY,
        BLOCKED_WAIT, // for notification
        BLOCKED_SUSPEND,
        BLOCKED_FAULT,
        IDLE, // kernel-only sentinel; never enters a scheduler ready queue
    };

    class process
    {
      public:
        // hardware_context is always top
        hardware_context registers {};
        alignas(a9n::WORD_BITS) floating_context floating_registers {};

        // for context-switch
        process_status status;
        a9n::sword     priority;
        a9n::sword     quantum;

        // for priority-scheduling
        a9n::word core_affinity { 0 };
        process  *preview { nullptr };
        process  *next { nullptr };
        bool      is_in_ready_queue { false };

        // for fault handling
        fault_type           fault_reason { fault_type::NONE };
        a9n::sword           fault_code;
        a9n::word            arch_fault_code;
        a9n::virtual_address fault_address;

        a9n::physical_address page_table; // TODO: remove

        // to root capability node
        capability_slot root_slot { /* .type = capability_type::NODE */ };

        // root address space
        capability_slot root_address_space { /* .type = capability_type::ADDRESS_SPACE, */ };

        // to ipc buffer
        capability_slot buffer_frame { /* .type = capability_type::FRAME */ };

        // to notification port
        capability_slot
            binded_notification_port { /* .type = capability_type::NOTIFICATION_PORT */ };

        // to resolver port
        capability_slot resolver_port { /* .type = capability_type::IPC_PORT */ };

        // buffer is *kernel* address (physical -> kernel (id))
        ipc_buffer *buffer { nullptr };

        // for IPC / notification
        process *next_ipc_queue { nullptr };
        process *preview_ipc_queue { nullptr };

        process *next_notification_queue { nullptr };
        process *preview_notification_queue { nullptr };

        // for notification mechanism
        class ipc_port          *current_ipc_port { nullptr };
        class notification_port *current_notification_port { nullptr };

        // When a sender is blocked waiting for a receiver, the receiver cannot know the sender's
        // identifier (which is natural, as it's slot-local!). Therefore, when blocking in this
        // state, the identifier needs to be temporarily saved here.
        a9n::word identifier_when_blocked;

        enum class source_reply_state_object : a9n::word
        {
            NONE,
            WAIT,
        } source_reply_state { source_reply_state_object::NONE };

        enum class destination_reply_state_object : a9n::word
        {
            NONE,
            READY_TO_REPLY,
        } destination_reply_state { destination_reply_state_object::NONE };

        // NOTE: *Why do we need source_reply_target?*
        // Suppose that process A is in the middle of a call to process B and A is destroyed (e.g.,
        // via Revoke/Remove). Although process B has A as the reply target, it will hold a pointer
        // to an invalid process (A in this case) that has already been destroyed.
        // Therefore, it is necessary to allow the caller to refer to the callee.
        process *source_reply_target { nullptr };
        process *destination_reply_target { nullptr };

        // tag for debugging
        char name[PROCESS_NAME_MAX];
    };

    static_assert(sizeof(process) <= 4096);
}

#endif
