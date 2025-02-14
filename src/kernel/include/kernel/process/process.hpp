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
        RUNNING,
        READY,
        BLOCKED
    };

    using process_id = a9n::sword;

    class process
    {
      public:
        // hardware_context is always top
        hardware_context registers;
        alignas(a9n::WORD_BITS) floating_context floating_registers;

        // for context-switch
        process_status status;
        a9n::sword     priority;
        a9n::sword     quantum;

        // for priority-scheduling
        a9n::word core_affinity;
        process  *preview;
        process  *next;

        // for fault handling
        fault_type           fault_reason { fault_type::NONE };
        a9n::sword           fault_code;
        a9n::virtual_address fault_address;

        a9n::physical_address page_table; // TODO: remove

        // to root capability node
        capability_slot root_slot { /* .type = capability_type::NODE */ };

        // root address space
        capability_slot root_address_space { /* .type = capability_type::ADDRESS_SPACE, */ };

        // to ipc buffer
        capability_slot buffer_frame { /* .type = capability_type::FRAME */ };

        // to resolver port
        capability_slot resolver_port { /* .type = capability_type::IPC_PORT */ };

        // buffer is *kernel* address (physical -> kernel (id))
        ipc_buffer *buffer;

        // for IPC / notification
        // TODO: rename to `*communication_queue_*`
        process *next_ipc_queue;
        process *preview_ipc_queue;

        // when destroy process, it should be removed from the queue
        enum class reply_state_object : a9n::word
        {
            NONE,
            WAIT,          // set when a reply is not found at `call` time
            READY_TO_REPLY // available for immediate `reply`
        };

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
        process *source_reply_target;
        process *destination_reply_target;

        // tag for debugging
        char name[PROCESS_NAME_MAX];

        /*=====remove_start=====*/
        process_id id;
        /*=====remove_end=====*/
    };

    static_assert(sizeof(process) <= 2048);
}

#endif
