#include "hal/interface/memory_manager.hpp"
#include "kernel/memory/memory_type.hpp"
#include <kernel/capability/generic.hpp>

#include <kernel/capability/address_space.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/capability/frame_capability.hpp>
#include <kernel/capability/io_port_capability.hpp>
#include <kernel/capability/ipc_port.hpp>
#include <kernel/capability/notification_port.hpp>
#include <kernel/capability/page_table_capability.hpp>
#include <kernel/capability/process_control_block.hpp>
#include <kernel/capability/virtual_cpu_capability.hpp>

#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/hal_result.hpp>
#include <hal/interface/process_manager.hpp>

#include <liba9n/common/calculate.hpp>

namespace a9n::kernel
{
    // helper
    namespace
    {
        capability_error convert_memory_error_to_capability_error(memory_error e)
        {
            switch (e)
            {
                using enum memory_error;

                case OUT_OF_MEMORY :
                    DEBUG_LOG("OUT_OF_MEMORY");
                    return capability_error::INVALID_ARGUMENT;

                case INVALID_ALIGNMENT :
                    DEBUG_LOG("INVALID_ALIGNMENT");
                    [[fallthrough]];
                case INVALID_ADDRESS :
                    DEBUG_LOG("INVALID_ADDRESS");
                    [[fallthrough]];
                default :
                    return capability_error::FATAL;
            }
        }
    }

    a9n::physical_address generic_info::base(void) const
    {
        return base_address;
    }

    // generic_info
    a9n::physical_address generic_info::current_watermark(void) const
    {
        return watermark;
    }

    bool generic_info::is_device() const
    {
        return device;
    }

    bool generic_info::is_allocatable(a9n::word memory_size_bits, a9n::word count) const
    {
        auto end_address        = base_address + size();
        auto request_unit_size  = (static_cast<a9n::word>(1) << memory_size_bits);
        auto aligned_watermark  = liba9n::align_value(watermark, request_unit_size);
        auto target_end_address = aligned_watermark + (request_unit_size * count);

        return (target_end_address <= end_address);
    }

    a9n::error generic_info::apply_allocate(a9n::word memory_size_bits)
    {
        auto request_unit_size = (static_cast<a9n::word>(1) << memory_size_bits);
        auto aligned_watermark = liba9n::align_value(watermark, request_unit_size);
        watermark              = aligned_watermark + request_unit_size;

        return 0;
    }

    memory_result<a9n::physical_address> generic_info::try_apply_allocate(a9n::word memory_size_bits)
    {
        auto original_watermark = current_watermark();
        auto unit_size          = (static_cast<a9n::word>(1) << memory_size_bits);
        auto aligned_watermark  = liba9n::align_value(watermark, unit_size);
        auto boundary_address   = aligned_watermark + (unit_size);

        if (boundary_address >= (base_address + size()))
        {
            return memory_error::OUT_OF_MEMORY;
        }

        apply_allocate(memory_size_bits);

        return aligned_watermark;
    }

    capability_slot_data generic_info::dump_slot_data() const
    {
        capability_slot_data data;

        data[0] = base_address;
        data[1] = serialize_generic_flags(device, size_bits);
        data[2] = watermark;

        return data;
    }

    // generic
    capability_result generic::execute(process &owner, capability_slot &self)
    {
        return decode_operation(owner, self);
    }

    capability_result generic::decode_operation(process &owner, capability_slot &self)
    {
        auto operation_type = [&](void) -> generic::operation_type
        {
            return static_cast<generic::operation_type>(
                a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (operation_type())
        {
            case CONVERT :
                {
                    DEBUG_LOG("generic::convert");
                    return convert(owner, self);
                }

            default :
                {
                    a9n::kernel::utility::logger::error("illegal operaton\n");
                    return capability_error::ILLEGAL_OPERATION;
                }
        }

        return {};
    }

    capability_result generic::convert(process &owner, capability_slot &self)
    {
        auto convert_capability_error = [](capability_lookup_error e) -> capability_error
        {
            switch (e)
            {
                using enum capability_lookup_error;

                case TERMINAL :
                    return capability_error::INVALID_DESCRIPTOR;

                case INDEX_OUT_OF_RANGE :
                    return capability_error::INVALID_ARGUMENT;

                case UNAVAILABLE :
                case EMPTY :
                    return capability_error::ILLEGAL_OPERATION;

                case UNEXPECTED :
                default :
                    return capability_error::FATAL;
            }
        };

        return retrieve_target_root_slot(owner)
            .transform_error(convert_capability_error)
            .and_then(
                [&](capability_slot *root_slot) -> capability_result
                {
                    auto self_info = create_generic_info(self.data);

                    // get index
                    auto      type          = capability_type::NONE;
                    a9n::word specific_bits = 0;
                    a9n::word count         = 0;
                    a9n::word index         = 0;

                    auto make_configure_mr_lambda =
                        [](process &owner, a9n::word &target_value, a9n::word index) -> decltype(auto)
                    {
                        return [&, index](void) -> a9n::hal::hal_result
                        {
                            return a9n::hal::get_message_register(owner, index)
                                .and_then(
                                    [&](a9n::word v) -> a9n::hal::hal_result
                                    {
                                        target_value = v;

                                        return {};
                                    }
                                );
                        };
                    };

                    a9n::word type_raw = 0;
                    auto      configure_capability_type
                        = make_configure_mr_lambda(owner, type_raw, CAPABILITY_TYPE);
                    auto configure_capability_size_bits
                        = make_configure_mr_lambda(owner, specific_bits, CAPABILITY_SPECIFIC_BITS);
                    auto configure_capability_count
                        = make_configure_mr_lambda(owner, count, CAPABILITY_COUNT);
                    auto configure_capability_index
                        = make_configure_mr_lambda(owner, index, SLOT_INDEX);

                    return configure_capability_type()
                        .and_then(configure_capability_size_bits)
                        .and_then(configure_capability_count)
                        .and_then(configure_capability_index)
                        .transform_error(
                            [&]([[maybe_unused]] a9n::hal::hal_error e) -> capability_error
                            {
                                return capability_error::FATAL;
                            }
                        )
                        .and_then(
                            [&](void) -> capability_result
                            {
                                [[unlikely]] if (count == 0 || count == 0)
                                {
                                    return capability_error::INVALID_ARGUMENT;
                                }

                                type = static_cast<capability_type>(type_raw);
                                [[unlikely]] if (self_info.is_device() && type != capability_type::FRAME)
                                {
                                    a9n::kernel::utility::logger::error(
                                        "device generic cannot be "
                                        "converted to anything "
                                        "other than frame"
                                    );
                                    return capability_error::INVALID_ARGUMENT;
                                }

                                auto memory_size_bits
                                    = calculate_capability_memory_size_bits(type, specific_bits)
                                          .unwrap_or(0);
                                if (!self_info.is_allocatable(memory_size_bits, count))
                                {
                                    a9n::kernel::utility::logger::error("out of memory");
                                    return capability_error::ILLEGAL_OPERATION;
                                }

                                // make
                                for (auto i = 0; i < count; i++)
                                {
                                    DEBUG_LOG("index : %llu", index + i);
                                    auto result
                                        = root_slot->component->retrieve_slot(index + i)
                                              .transform_error(convert_capability_error)
                                              .and_then(
                                                  [&](capability_slot *destination_slot) -> capability_result
                                                  {
                                                      if (destination_slot->type
                                                          != capability_type::NONE)
                                                      {
                                                          // clang-format off
                                                          a9n::kernel::utility::logger::printk("slot type : 0x%llx\n", destination_slot->type);
                                                          a9n::kernel::utility::logger::error("slot is already used");
                                                          // clang-format on
                                                          return capability_error::INVALID_ARGUMENT;
                                                      }

                                                      DEBUG_LOG(
                                                          "convert to 0x%llx (destination)",
                                                          destination_slot
                                                      );

                                                      return destination_slot->try_remove_and_init()
                                                          .transform_error(
                                                              [](kernel_error e) -> capability_error
                                                              {
                                                                  return capability_error::FATAL;
                                                              }
                                                          )
                                                          .and_then(
                                                              [&](void) -> capability_result
                                                              {
                                                                  return try_make_capability(
                                                                      type,
                                                                      memory_size_bits,
                                                                      specific_bits,
                                                                      self_info,
                                                                      self,
                                                                      *destination_slot
                                                                  );
                                                              }
                                                          )
                                                          .and_then(
                                                              [&](void) -> capability_result
                                                              {
                                                                  DEBUG_LOG(
                                                                      "convert [slot : "
                                                                      "0x%llx][type : "
                                                                      "0x%4x][component : 0x%llx]",
                                                                      destination_slot,
                                                                      type,
                                                                      destination_slot->component
                                                                  );
                                                                  // update status
                                                                  self.data
                                                                      = self_info.dump_slot_data();
                                                                  self.insert_child(*destination_slot);

                                                                  return {};
                                                              }
                                                          );
                                                  }
                                              );
                                    if (!result)
                                    {
                                        return result.unwrap_error();
                                    }

                                    DEBUG_LOG(
                                        "[base : 0x%llx][watermark : 0x%llx]",
                                        self_info.base(),
                                        self_info.current_watermark()
                                    );
                                }

                                return {};
                            }
                        );
                }
            );
    }

    generic_info generic::create_generic_info(const capability_slot_data &data)
    {
        return generic_info {
            data[0],
            generic_flags_size_bits(data[1]),
            generic_flags_is_device(data[1]),
            data[2],
        };
    }

    capability_lookup_result generic::retrieve_target_root_slot(process &owner) const
    {
        a9n::word target_descriptor = 0;
        a9n::word target_depth      = 0;

        return a9n::hal::get_message_register(owner, ROOT_DESCRIPTOR)
            .and_then(
                [&](a9n::word descriptor) -> a9n::hal::hal_result
                {
                    target_descriptor = descriptor;
                    target_depth      = extract_depth(descriptor);
                    DEBUG_LOG(
                        "descriptor : 0x%016llx   target_depth : "
                        "0x%016llx",
                        target_descriptor,
                        target_depth
                    );

                    return {};
                }
            )
            .transform_error(
                []([[maybe_unused]] a9n::hal::hal_error e) -> capability_lookup_error
                {
                    return capability_lookup_error::UNEXPECTED;
                }
            )
            .and_then(
                [&](void) -> capability_lookup_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(target_descriptor, target_depth, a9n::BYTE_BITS);
                }
            );
    }

    constexpr liba9n::option<a9n::word>
        generic::calculate_capability_memory_size_bits(capability_type type, a9n::word specific_bits)
    {
        DEBUG_LOG("calculate_capability_memory_size_bits : 0x%llx, 0x%llx", type, specific_bits);
        switch (type)
        {
            using enum capability_type;

            case NONE :
                return liba9n::option_none;

            case DEBUG :
                return liba9n::option_none;

            case NODE :
                return liba9n::calculate_radix_ceil(
                    liba9n::calculate_radix_ceil(sizeof(capability_node))
                    + liba9n::calculate_radix_ceil(sizeof(capability_slot) * specific_bits)
                );

            case GENERIC :
                return specific_bits;

            case PAGE_TABLE :
                return liba9n::calculate_radix(a9n::PAGE_SIZE);

            case FRAME :
                return liba9n::calculate_radix(a9n::PAGE_SIZE);

            case PROCESS_CONTROL_BLOCK :
                return liba9n::calculate_radix_ceil(sizeof(process_control_block));

            case IPC_PORT :
                return liba9n::calculate_radix_ceil(sizeof(ipc_port));

            case NOTIFICATION_PORT :
                return liba9n::calculate_radix_ceil(sizeof(notification_port));

            case INTERRUPT_REGION :
            case INTERRUPT_PORT :
                return liba9n::option_none;

            case IO_PORT :
                return liba9n::calculate_radix_ceil(sizeof(io_port_capability));

            case VIRTUAL_CPU :
                return liba9n::calculate_radix_ceil(sizeof(virtual_cpu_capability));

            case VIRTUAL_PAGE_TABLE :
            default :
                return liba9n::option_none;
        }
    }

    capability_result generic::revoke(capability_slot &self)
    {
        // generic cannot be copied; so it is basically read/write and 1-set
        [[unlikely]] if (!(self.rights & capability_slot::object_rights::READ)
                         && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        // revoke recursively
        for (auto start_slot = self.next_slot; start_slot->depth < self.depth;
             start_slot      = start_slot->next_slot)
        {
            if (!start_slot->component)
            {
                return {};
            }

            auto result = start_slot->component->revoke(*start_slot);
            if (!result)
            {
                return result;
            }
        }

        return {};
    }

    capability_result generic::try_make_capability(
        capability_type  type,
        a9n::word        memory_size_bits,
        a9n::word        specific_bits,
        generic_info    &info,
        capability_slot &self,
        capability_slot &target_slot
    )
    {
        switch (type)
        {
            using enum capability_type;

            case NONE :
                a9n::kernel::utility::logger::error("invalid capability type");
                return capability_error::ILLEGAL_OPERATION;

            case DEBUG :
                return capability_error::DEBUG_UNIMPLEMENTED;

            case NODE :
                // specific_bits is radix
                DEBUG_LOG("convert to node");
                return info.try_apply_allocate(memory_size_bits)
                    .transform_error(convert_memory_error_to_capability_error)
                    .and_then(
                        [&](a9n::physical_address new_watermark) -> capability_result
                        {
                            return try_make_capability_node(
                                       physical_to_virtual_address(new_watermark),
                                       specific_bits
                            )
                                .and_then(
                                    [&](liba9n::not_null<capability_node> node) -> kernel_result
                                    {
                                        return try_configure_capability_node_slot(
                                            target_slot,
                                            node.get()
                                        );
                                    }
                                )
                                .transform_error(
                                    [](kernel_error e) -> capability_error
                                    {
                                        return capability_error::FATAL;
                                    }
                                );
                        }
                    );

            case GENERIC :
                {
                    DEBUG_LOG("convert to generic");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto child_info = generic_info {
                                    new_watermark,
                                    memory_size_bits,
                                    info.is_device(),
                                    new_watermark
                                };

                                return try_configure_generic_slot(self, info)
                                    .and_then(
                                        [&](void) -> kernel_result
                                        {
                                            return try_configure_generic_slot(target_slot, child_info);
                                        }
                                    )
                                    .transform_error(
                                        [](kernel_error e) -> capability_error
                                        {
                                            return capability_error::FATAL;
                                        }
                                    );
                            }
                        );
                }

            case ADDRESS_SPACE :
                {
                    DEBUG_LOG("convert to address_space");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                return a9n::hal::make_address_space(new_watermark)
                                    .transform_error(
                                        [](a9n::hal::hal_error e) -> capability_error
                                        {
                                            return capability_error::FATAL;
                                        }
                                    )
                                    .and_then(
                                        [&](a9n::kernel::page_table table) -> capability_result
                                        {
                                            return try_configure_address_space_slot(target_slot, table)
                                                .transform_error(
                                                    [](kernel_error e) -> capability_error
                                                    {
                                                        return capability_error::FATAL;
                                                    }
                                                );
                                        }
                                    );
                            }
                        );
                }

            case PAGE_TABLE :
                {
                    DEBUG_LOG("convert to page_table");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto target_table
                                    = page_table { .address = new_watermark, .depth = specific_bits };

                                return try_configure_page_table_slot(target_slot, target_table)
                                    .transform_error(
                                        [](kernel_error e) -> capability_error
                                        {
                                            return capability_error::FATAL;
                                        }
                                    );
                            }
                        );
                }

            case FRAME :
                {
                    DEBUG_LOG("convert to frame");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto target_frame = frame { .address = new_watermark };

                                return try_configure_frame_slot(target_slot, target_frame)
                                    .transform_error(
                                        [](kernel_error e) -> capability_error
                                        {
                                            return capability_error::FATAL;
                                        }
                                    );
                            }
                        );
                }

            case PROCESS_CONTROL_BLOCK :
                {
                    DEBUG_LOG("convert to process_control_block");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto pcb = new (
                                    a9n::kernel::physical_to_virtual_pointer<void *>(new_watermark)
                                ) process_control_block {};

                                return try_configure_process_control_block_slot(target_slot, *pcb)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }

            case IPC_PORT :
                {
                    DEBUG_LOG("convert to ipc_port");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto port = new (
                                    a9n::kernel::physical_to_virtual_pointer<void *>(new_watermark)
                                ) ipc_port {};

                                return try_configure_ipc_port_slot(target_slot, *port, 0)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }

            case NOTIFICATION_PORT :
                {
                    DEBUG_LOG("convert to notification_port");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto port = new (
                                    a9n::kernel::physical_to_virtual_pointer<void *>(new_watermark)
                                ) notification_port {};

                                return try_configure_notification_port_slot(target_slot, *port, 0)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }
            case INTERRUPT_REGION :
                DEBUG_LOG("convert to interrupt_region");
                return capability_error::DEBUG_UNIMPLEMENTED;

            case INTERRUPT_PORT :
                DEBUG_LOG("convert to interrupt_port");
                return capability_error::DEBUG_UNIMPLEMENTED;

            case IO_PORT :
                DEBUG_LOG("convert to io_port");
                return info.try_apply_allocate(memory_size_bits)
                    .transform_error(convert_memory_error_to_capability_error)
                    .and_then(
                        [&](a9n::physical_address new_watermark) -> capability_result
                        {
                            auto port = new (
                                a9n::kernel::physical_to_virtual_pointer<void *>(new_watermark)
                            ) io_port_capability {};
                            auto caution_test_address_range
                                = io_port_address_range { .min = 0, .max = ~static_cast<a9n::word>(0) };

                            return try_configure_io_port_slot(target_slot, *port, caution_test_address_range)
                                .transform_error(convert_kernel_to_capability_error);
                        }
                    );

            case VIRTUAL_CPU :
                {
                    DEBUG_LOG("convert to virtual_cpu");
                    return info.try_apply_allocate(memory_size_bits)
                        .transform_error(convert_memory_error_to_capability_error)
                        .and_then(
                            [&](a9n::physical_address new_watermark) -> capability_result
                            {
                                auto virtual_cpu = new (
                                    a9n::kernel::physical_to_virtual_pointer<void *>(new_watermark)
                                ) virtual_cpu_capability {};

                                return try_configure_virtual_cpu_slot(target_slot, *virtual_cpu)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }

            case VIRTUAL_PAGE_TABLE :
                DEBUG_LOG("convert to virtual_page_table");
                return capability_error::DEBUG_UNIMPLEMENTED;

            default :
                return capability_error::DEBUG_UNIMPLEMENTED;
        }
    }

}
