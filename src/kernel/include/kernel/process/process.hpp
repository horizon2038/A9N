#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <hal/arch/arch_types.hpp>
#include <kernel/types.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
// #include <kernel/memory/address_space_object.hpp>

#include <stdint.h>

namespace a9n::kernel
{
    inline constexpr a9n::word QUANTUM_MAX      = 10;
    inline constexpr a9n::word STACK_SIZE_MAX   = 8192;
    inline constexpr a9n::word PROCESS_NAME_MAX = 128;

    using hardware_context = liba9n::std::array<a9n::word, a9n::hal::HARDWARE_CONTEXT_SIZE>;

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

        // for context-switch
        process_status status;
        a9n::sword     priority;
        a9n::sword     quantum;

        // for priority-scheduling
        process *preview;
        process *next;

        a9n::physical_address page_table;

        // to root capability node
        capability_slot root_slot { .type = capability_type::NODE };

        // root address space
        capability_slot root_address_space {
            .type = capability_type::PAGE_TABLE,
        };

        // to ipc buffer
        capability_slot buffer_frame { .type = capability_type::FRAME };

        // to resolver port
        capability_slot resolver_port { .type = capability_type::IPC_PORT };

        // buffer is *kernel* address (physical -> kernel (id))
        ipc_buffer *buffer;
        process    *next_ipc_queue;
        process    *preview_ipc_queue;

        // tag for debugging
        char name[PROCESS_NAME_MAX];

        /*=====remove_start=====*/
        process_id id;
        /*=====remove_end=====*/
    };

}

#endif
