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
        struct configuration_info
        {
            a9n::word data;

            constexpr configuration_info(a9n::word data) : data(data)
            {
            }

            constexpr configuration_info(
                bool address_space,
                bool root_node,
                bool frame_ipc_buffer,
                bool notification_port,
                bool ipc_port_resolver,
                bool instruction_pointer,
                bool stack_pointer,
                bool thread_local_base,
                bool priority,
                bool affinity
            )
            {
                // configure space
                configure_address_space(address_space);
                configure_root_node(root_node);

                // configure ipc buffer
                configure_frame_ipc_buffer(frame_ipc_buffer);

                // configure notification
                configure_notification_port(notification_port);

                // configure resolver (fault handler)
                configure_ipc_port_resolver(ipc_port_resolver);

                // configure hardware-independent registers
                configure_instruction_pointer(instruction_pointer);
                configure_stack_pointer(stack_pointer);
                configure_thread_local_base(thread_local_base);

                // scheduling
                configure_priority(priority); // priority
                configure_affinity(affinity); // SMP only
            }

            constexpr void configure_address_space(bool address_space)
            {
                data = (data & ~(1 << 0)) | address_space;
            }

            constexpr void configure_root_node(bool root_node)
            {
                data = (data & ~(1 << 1)) | root_node;
            }

            constexpr void configure_frame_ipc_buffer(bool frame_ipc_buffer)
            {
                data = (data & ~(1 << 2)) | frame_ipc_buffer;
            }

            constexpr void configure_notification_port(bool notification_port)
            {
                data = (data & ~(1 << 3)) | notification_port;
            }

            constexpr void configure_ipc_port_resolver(bool ipc_port_resolver)
            {
                data = (data & ~(1 << 4)) | ipc_port_resolver;
            }

            constexpr void configure_instruction_pointer(bool instruction_pointer)
            {
                data = (data & ~(1 << 5)) | instruction_pointer;
            }

            constexpr void configure_stack_pointer(bool stack_pointer)
            {
                data = (data & ~(1 << 6)) | stack_pointer;
            }

            constexpr void configure_thread_local_base(bool thread_local_base)
            {
                data = (data & ~(1 << 7)) | thread_local_base;
            }

            constexpr void configure_priority(bool priority)
            {
                data = (data & ~(1 << 8)) | priority;
            }

            constexpr void configure_affinity(bool affinity)
            {
                data = (data & ~(1 << 9)) | affinity;
            }

            constexpr bool is_address_space(void) const
            {
                return data & (1 << 0);
            }

            constexpr bool is_root_node(void) const
            {
                return data & (1 << 1);
            }

            constexpr bool is_frame_ipc_buffer(void) const
            {
                return data & (1 << 2);
            }

            constexpr bool is_notification_port(void) const
            {
                return data & (1 << 3);
            }

            constexpr bool is_ipc_port_resolver(void) const
            {
                return data & (1 << 4);
            }

            constexpr bool is_instruction_pointer(void) const
            {
                return data & (1 << 5);
            }

            constexpr bool is_stack_pointer(void) const
            {
                return data & (1 << 6);
            }

            constexpr bool is_thread_local_base(void) const
            {
                return data & (1 << 7);
            }

            constexpr bool is_priority(void) const
            {
                return data & (1 << 8);
            }

            constexpr bool is_affinity(void) const
            {
                return data & (1 << 9);
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
            NOTIFICATION_PORT,

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
