#ifndef A9N_KERNEL_CAPABILITY_PROCESS_CONTROL_BLOCK_HPP
#define A9N_KERNEL_CAPABILITY_PROCESS_CONTROL_BLOCK_HPP

#include "kernel/types.hpp"
#include <hal/arch/arch_types.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/process_manager.hpp>

#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    class process_control_block final : public capability_component
    {
      private:
        capability_result cancel_own_wait(void);

        // Selection bits in the public PCB CONFIGURE message, not register indices.
        enum class configuration_flag : a9n::word
        {
            ADDRESS_SPACE       = a9n::word { 1 } << 0,
            ROOT_NODE           = a9n::word { 1 } << 1,
            FRAME_IPC_BUFFER    = a9n::word { 1 } << 2,
            NOTIFICATION_PORT   = a9n::word { 1 } << 3,
            IPC_PORT_RESOLVER   = a9n::word { 1 } << 4,
            INSTRUCTION_POINTER = a9n::word { 1 } << 5,
            STACK_POINTER       = a9n::word { 1 } << 6,
            THREAD_LOCAL_BASE   = a9n::word { 1 } << 7,
            PRIORITY            = a9n::word { 1 } << 8,
            AFFINITY            = a9n::word { 1 } << 9,
        };

        struct configuration_info
        {
            a9n::word flags;

            constexpr bool contains(configuration_flag flag) const
            {
                return (flags & static_cast<a9n::word>(flag)) != 0;
            }
        };

        enum operation_index : a9n::word
        {
            RESERVED,
            OPERATION_TYPE,
        };

        enum result_index : a9n::word
        {
            IS_SUCCESS,
            ERROR_CODE,
            RESERVED_FOR_REGISTER_COUNT,
            REGISTER,
        };

        enum operation_configure_index : a9n::word
        {
            CONFIGURE_RESERVED = OPERATION_TYPE,

            CONFIGURATION_INFO,

            // space
            ADDRESS_SPACE_DESCRIPTOR,
            ROOT_NODE_DESCRIPTOR,

            // ipc
            FRAME_IPC_BUFFER_DESCRIPTOR,
            NOTIFICATION_PORT_DESCRIPTOR,

            // resolver (fault handler)
            IPC_PORT_RESOLVER,

            // general purpose registers
            INSTRUCTION_POINTER,
            STACK_POINTER,
            THREAD_LOCAL_BASE,

            PRIORITY,
            AFFINITY, // SMP only
        };

        enum operation_read_write_register_index : a9n::word
        {
            READ_WRITE_REGISTER_RESERVED = OPERATION_TYPE,

            REGISTER_COUNT,
        };

        enum operation_type : a9n::word
        {
            NONE,
            CONFIGURE,
            READ_REGISTER,
            WRITE_REGISTER,
            RESUME,
            SUSPEND,
        };

      public:
        process process_core {};

        process_control_block(void);

        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_configure(process &owner, capability_slot &self);
        capability_result
            configure_address_space(process &owner, a9n::capability_descriptor descriptor);
        capability_result configure_root_node(process &owner, a9n::capability_descriptor descriptor);
        capability_result
            configure_frame_ipc_buffer(process &owner, a9n::capability_descriptor descriptor);
        capability_result
            configure_notification_port(process &owner, a9n::capability_descriptor descriptor);
        capability_result
            configure_ipc_port_resolver(process &owner, a9n::capability_descriptor descriptor);
        capability_result operation_read_register(process &owner, capability_slot &self);
        capability_result operation_write_register(process &owner, capability_slot &self);
        capability_result operation_resume(process &owner, capability_slot &self);
        capability_result operation_suspend(process &owner, capability_slot &self);
        capability_result detach_from_wait_queues(void);

        capability_result revoke(capability_slot &self) override;

        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            return capability_lookup_error::TERMINAL;
        }

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) override
        {
            return capability_lookup_error::TERMINAL;
        }
    };

    inline kernel_result
        try_configure_process_control_block_slot(capability_slot &slot, process_control_block &pcb)
    {
        slot.init();
        slot.rights    = capability_slot::object_rights::ALL;
        slot.component = &pcb;
        slot.type      = capability_type::PROCESS_CONTROL_BLOCK;
        slot.data.fill(0);

        return {};
    }
}

#endif
