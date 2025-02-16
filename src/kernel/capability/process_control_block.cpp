#include "kernel/process/process.hpp"
#include <kernel/capability/process_control_block.hpp>

#include <hal/interface/process_manager.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    process_control_block::process_control_block()
    {
        // init hardware-specific contexts
        hal::init_hardware_context(hal::cpu_mode::USER, process_core.registers);
    }

    capability_result process_control_block::execute(process &owner, capability_slot &self)
    {
        auto target_operation = [&](void) -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (target_operation())
        {
            case operation_type::CONFIGURE :
                DEBUG_LOG("process_control_block::configure");
                return operation_configure(owner, self);

            case operation_type::READ_REGISTER :
                DEBUG_LOG("process_control_block::read_register");
                return operation_read_register(owner, self);

            case operation_type::WRITE_REGISTER :
                DEBUG_LOG("process_control_block::write_register");
                return operation_write_register(owner, self);

            case operation_type::RESUME :
                DEBUG_LOG("process_control_block::resume");
                return operation_resume(owner, self);

            case operation_type::SUSPEND :
                DEBUG_LOG("process_control_block::suspend");
                return operation_suspend(owner, self);

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    };

    capability_result process_control_block::operation_configure(process &owner, capability_slot &self)
    {
        return a9n::hal::get_message_register(owner, CONFIGURATION_INFO)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error)
            .transform(
                [](a9n::word v) -> configuration_info
                {
                    return configuration_info(v);
                }
            )
            .and_then(
                [&](configuration_info info) -> capability_result
                {
                    auto get_argument =
                        [&](a9n::word index) -> liba9n::result<a9n::word, capability_error>
                    {
                        return a9n::hal::get_message_register(owner, index)
                            .transform_error(convert_hal_to_kernel_error)
                            .transform_error(convert_kernel_to_capability_error);
                    };

                    if (info.is_address_space())
                    {
                        DEBUG_LOG("process_control_block::configure::address_space");
                        auto result
                            = get_argument(ADDRESS_SPACE_DESCRIPTOR)
                                  .and_then(
                                      [&](a9n::word descriptor) -> capability_result
                                      {
                                          return configure_address_space(owner, descriptor);
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_root_node())
                    {
                        DEBUG_LOG("process_control_block::configure::root_node");
                        auto result
                            = get_argument(ROOT_NODE_DESCRIPTOR)
                                  .and_then(
                                      [&](a9n::word descriptor) -> capability_result
                                      {
                                          return configure_root_node(owner, descriptor);
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_frame_ipc_buffer())
                    {
                        DEBUG_LOG("process_control_block::configure::frame_ipc_buffer");
                        auto result
                            = get_argument(FRAME_IPC_BUFFER_DESCRIPTOR)
                                  .and_then(
                                      [&](a9n::word descriptor) -> capability_result
                                      {
                                          return configure_frame_ipc_buffer(owner, descriptor);
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_notification_port())
                    {
                        DEBUG_LOG("process_control_block::configure::notification_port");
                        return capability_error::DEBUG_UNIMPLEMENTED;
                    }

                    if (info.is_ipc_port_resolver())
                    {
                        DEBUG_LOG("process_control_block::configure::ipc_port_resolver");
                        auto result
                            = get_argument(IPC_PORT_RESOLVER)
                                  .and_then(
                                      [&](a9n::word descriptor) -> capability_result
                                      {
                                          return configure_ipc_port_resolver(owner, descriptor);
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_instruction_pointer())
                    {
                        DEBUG_LOG("process_control_block::configure::instruction_pointer");
                        auto result
                            = a9n::hal::get_message_register(owner, INSTRUCTION_POINTER)
                                  .transform_error(convert_hal_to_kernel_error)
                                  .transform_error(convert_kernel_to_capability_error)
                                  .and_then(
                                      [&](a9n::word value) -> capability_result
                                      {
                                          if (!a9n::hal::is_valid_user_address(value))
                                          {
                                              return capability_error::ILLEGAL_OPERATION;
                                          }

                                          DEBUG_LOG("instruction_pointer : 0x%016llx", value);

                                          auto result = a9n::hal::configure_general_register(
                                              process_core,
                                              a9n::hal::register_type::INSTRUCTION_POINTER,
                                              value
                                          );
                                          if (!result)
                                          {
                                              return capability_error::FATAL;
                                          }

                                          return {};
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_stack_pointer())
                    {
                        DEBUG_LOG("process_control_block::configure::stack_pointer");
                        auto result
                            = a9n::hal::get_message_register(owner, STACK_POINTER)
                                  .transform_error(convert_hal_to_kernel_error)
                                  .transform_error(convert_kernel_to_capability_error)
                                  .and_then(
                                      [&](a9n::word value) -> capability_result
                                      {
                                          if (!a9n::hal::is_valid_user_address(value))
                                          {
                                              return capability_error::ILLEGAL_OPERATION;
                                          }

                                          DEBUG_LOG("stack_pointer : 0x%016llx", value);

                                          auto result = a9n::hal::configure_general_register(
                                              process_core,
                                              a9n::hal::register_type::STACK_POINTER,
                                              value
                                          );
                                          if (!result)
                                          {
                                              return capability_error::FATAL;
                                          }

                                          return {};
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_thread_local_base())
                    {
                        DEBUG_LOG("process_control_block::configure::thread_local_base");
                        auto result
                            = a9n::hal::get_message_register(owner, THREAD_LOCAL_BASE)
                                  .transform_error(convert_hal_to_kernel_error)
                                  .transform_error(convert_kernel_to_capability_error)
                                  .and_then(
                                      [&](a9n::word value) -> capability_result
                                      {
                                          if (!a9n::hal::is_valid_user_address(value))
                                          {
                                              return capability_error::ILLEGAL_OPERATION;
                                          }

                                          auto result = a9n::hal::configure_general_register(
                                              process_core,
                                              a9n::hal::register_type::THREAD_LOCAL_BASE,
                                              value
                                          );
                                          if (!result)
                                          {
                                              return capability_error::FATAL;
                                          }

                                          return {};
                                      }
                                  );
                        if (!result)
                        {
                            return capability_error::FATAL;
                        }
                    }

                    if (info.is_priority())
                    {
                        DEBUG_LOG("process_control_block::configure::priority");
                        // process_core.priority = self.data[0];
                        auto result
                            = a9n::hal::get_message_register(owner, PRIORITY)
                                  .transform_error(convert_hal_to_kernel_error)
                                  .transform_error(convert_kernel_to_capability_error)
                                  .and_then(
                                      [&](a9n::word priority) -> capability_result
                                      {
                                          if (priority >= PRIORITY_MAX)
                                          {
                                              // range error !
                                              return capability_error::INVALID_ARGUMENT;
                                          }

                                          process_core.priority = priority;

                                          return {};
                                      }
                                  );
                        if (!result)
                        {
                            return result;
                        }
                    }

                    if (info.is_affinity())
                    {
                        DEBUG_LOG("process_control_block::configure::affinity");
                        // process_core.core_affinity = self.data[0];
                        process_core.core_affinity = a9n::hal::get_message_register(owner, AFFINITY)
                                                         .unwrap_or(static_cast<a9n::word>(0));
                    }

                    return {};
                }
            );
    }

    capability_result process_control_block::configure_address_space(
        process                   &owner,
        a9n::capability_descriptor descriptor
    )
    {
        auto convert_lookup_to_capability_error = [](capability_lookup_error e) -> capability_error
        {
            return capability_error::INVALID_DESCRIPTOR;
        };

        return owner.root_slot.component
            ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    DEBUG_LOG("configure_address_space");
                    if (slot->type != capability_type::ADDRESS_SPACE)
                    {
                        return capability_error::INVALID_DESCRIPTOR;
                    }

                    auto result = process_core.root_address_space.try_remove_and_init();
                    if (!result)
                    {
                        DEBUG_LOG("error : %s", kernel_error_to_string(result.unwrap_error()));
                        a9n::kernel::utility::logger::error("failed to remove the address space");
                        DEBUG_LOG(
                            "rights : 0x%llx",
                            static_cast<a9n::word>(process_core.root_address_space.rights)
                        );
                        return capability_error::FATAL;
                    }

                    DEBUG_LOG("copy address space");
                    return try_copy_capability_slot(process_core.root_address_space, *slot)
                        .transform_error(
                            [&](kernel_error e) -> capability_error
                            {
                                if (e == kernel_error::INIT_FIRST)
                                {
                                    DEBUG_LOG("init first");
                                    return capability_error::INVALID_DESCRIPTOR;
                                }

                                if (e == kernel_error::PERMISSION_DENIED)
                                {
                                    DEBUG_LOG("permission denied");
                                    DEBUG_LOG("rights : 0x%llx", static_cast<a9n::word>(slot->rights));
                                    return capability_error::PERMISSION_DENIED;
                                }

                                return capability_error::INVALID_ARGUMENT;
                            }
                        );
                }
            );
    }

    capability_result
        process_control_block::configure_root_node(process &owner, a9n::capability_descriptor descriptor)
    {
        auto convert_lookup_to_capability_error = [](capability_lookup_error e) -> capability_error
        {
            return capability_error::INVALID_DESCRIPTOR;
        };

        return owner.root_slot.component
            ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    if (slot->type != capability_type::NODE)
                    {
                        return capability_error::INVALID_DESCRIPTOR;
                    }

                    auto result = process_core.root_slot.try_remove_and_init();
                    if (!result)
                    {
                        return capability_error::FATAL;
                    }

                    return try_copy_capability_slot(process_core.root_slot, *slot)
                        .transform_error(convert_kernel_to_capability_error);
                }
            );
    }

    capability_result process_control_block::configure_frame_ipc_buffer(
        process                   &owner,
        a9n::capability_descriptor descriptor
    )
    {
        auto convert_lookup_to_capability_error = [](capability_lookup_error e) -> capability_error
        {
            return capability_error::INVALID_DESCRIPTOR;
        };

        return owner.root_slot.component
            ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    if (slot->type != capability_type::FRAME)
                    {
                        return capability_error::INVALID_DESCRIPTOR;
                    }

                    auto result = process_core.buffer_frame.try_remove_and_init();
                    if (!result)
                    {
                        return capability_error::FATAL;
                    }

                    return try_copy_capability_slot(process_core.buffer_frame, *slot)
                        .transform_error(convert_kernel_to_capability_error);
                }
            );
    }

    capability_result process_control_block::configure_notification_port(
        process                   &owner,
        a9n::capability_descriptor descriptor
    )
    {
        return capability_error::DEBUG_UNIMPLEMENTED;
    }

    capability_result process_control_block::configure_ipc_port_resolver(
        process                   &owner,
        a9n::capability_descriptor descriptor
    )
    {
        auto convert_lookup_to_capability_error = [](capability_lookup_error e) -> capability_error
        {
            return capability_error::INVALID_DESCRIPTOR;
        };

        return owner.root_slot.component
            ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    if (slot->type != capability_type::IPC_PORT)
                    {
                        return capability_error::INVALID_DESCRIPTOR;
                    }

                    auto result = slot->try_remove_and_init();
                    if (!result)
                    {
                        return capability_error::FATAL;
                    }

                    return try_copy_capability_slot(process_core.resolver_port, *slot)
                        .transform_error(convert_kernel_to_capability_error);
                }
            );
    }

    capability_result
        process_control_block::operation_read_register(process &owner, capability_slot &self)

    {
        return a9n::hal::get_message_register(owner, REGISTER_COUNT)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](a9n::word register_count) -> capability_result
                {
                    if (register_count > a9n::hal::HARDWARE_CONTEXT_SIZE)
                    {
                        return capability_error::INVALID_ARGUMENT;
                    }

                    // TODO: reserve status region
                    for (auto i = 0; i < register_count; i++)
                    {
                        auto result = a9n::hal::configure_message_register(
                            owner,
                            i + REGISTER,
                            process_core.registers[i]
                        );
                        if (!result)
                        {
                            return capability_error::FATAL;
                        }
                    }

                    return {};
                }
            );
    }

    capability_result
        process_control_block::operation_write_register(process &owner, capability_slot &self)
    {
        return a9n::hal::get_message_register(owner, REGISTER_COUNT)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](a9n::word register_count) -> capability_result
                {
                    if (register_count > a9n::hal::HARDWARE_CONTEXT_SIZE)
                    {
                        return capability_error::INVALID_ARGUMENT;
                    }

                    for (auto i = 0; i < register_count; i++)
                    {
                        auto result
                            = a9n::hal::get_message_register(owner, i + REGISTER)
                                  .transform_error(convert_hal_to_kernel_error)
                                  .transform_error(convert_kernel_to_capability_error)
                                  .and_then(
                                      [&](a9n::word value) -> capability_result
                                      {
                                          process_core.registers[i] = value;

                                          return {};
                                      }
                                  );

                        if (!result)
                        {
                            return capability_error::FATAL;
                        }
                    }

                    return {};
                }
            );

        return {};
    }

    capability_result process_control_block::operation_resume(process &owner, capability_slot &self)
    {
        process_core.status = process_status::READY;

        return process_manager_core
            .mark_scheduled(process_core)
            /*
            .and_then(
                [&](void) -> kernel_result
                {
                    return process_manager_core.try_schedule_and_switch();
                }
            )
            */
            .transform_error(convert_kernel_to_capability_error);
    }

    capability_result process_control_block::operation_suspend(process &owner, capability_slot &self)
    {
        process_core.status   = process_status::BLOCKED;
        process_core.priority = 0;

        return {};
    }

    capability_result process_control_block::revoke(capability_slot &self)
    {
        return process_core.root_slot.try_remove_and_init()
            .and_then(
                [&](void) -> kernel_result
                {
                    return process_core.root_address_space.try_remove_and_init();
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    return process_core.buffer_frame.try_remove_and_init();
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    return process_core.resolver_port.try_remove_and_init();
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    a9n::hal::init_hardware_context(hal::cpu_mode::USER, process_core.registers);
                    return {};
                }
            )
            .transform_error(convert_kernel_to_capability_error);
    }
}
