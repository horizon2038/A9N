#include "hal/hal_result.hpp"
#include "kernel/capability/capability_component.hpp"
#include <kernel/capability/generic.hpp>

#include <kernel/capability/capability_factory.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/capability/operation/generic_operation.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/process_manager.hpp>

#include <liba9n/common/calculate.hpp>

#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/frame_capability.hpp>
#include <kernel/capability/ipc_port.hpp>
#include <kernel/capability/page_table_capability.hpp>
#include <kernel/capability/process_control_block.hpp>

namespace a9n::kernel
{
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

        return (target_end_address < end_address);
    }

    a9n::error generic_info::apply_allocate(a9n::word memory_size_bits)
    {
        auto request_unit_size = (static_cast<a9n::word>(1) << memory_size_bits);
        auto aligned_watermark = liba9n::align_value(watermark, request_unit_size);
        watermark              = aligned_watermark + request_unit_size;

        return 0;
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
        switch (auto operation_type = static_cast<generic_operation>(
                    a9n::hal::get_message_register(owner, generic_convert_argument::OPERATION_TYPE)
                        .unwrap_or(static_cast<a9n::word>(0))
                ))
        {
            using enum generic_operation;

            case CONVERT :
                {
                    a9n::kernel::utility::logger::printk("generic::convert\n");
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
        namespace argument            = generic_convert_argument;
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
                [&](capability_slot *slot) -> capability_result
                {
                    auto self_info = create_generic_info(self.data);

                    // get index
                    auto      type      = capability_type::NONE;
                    a9n::word size_bits = 0;
                    a9n::word count     = 0;

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
                        = make_configure_mr_lambda(owner, type_raw, argument::CAPABILITY_TYPE);
                    auto configure_capability_size_bits
                        = make_configure_mr_lambda(owner, size_bits, argument::CAPABILITY_SIZE_BITS);
                    auto configure_capability_count
                        = make_configure_mr_lambda(owner, count, argument::CAPABILITY_COUNT);

                    return configure_capability_type()
                        .and_then(configure_capability_size_bits)
                        .and_then(configure_capability_count)
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
                                    return capability_error::INVALID_ARGUMENT;
                                }

                                auto memory_size_bits
                                    = calculate_capability_memory_size_bits(type, size_bits).unwrap_or(0);
                                if (!self_info.is_allocatable(memory_size_bits, count))
                                {
                                    return capability_error::ILLEGAL_OPERATION;
                                }

                                // make
                                for (auto i = 0; i < count; i++)
                                {
                                    auto result
                                        = slot->component->retrieve_slot(i)
                                              .transform_error(convert_capability_error)
                                              .and_then(
                                                  [&](capability_slot *slot) -> capability_result
                                                  {
                                                      return try_make_capability(
                                                          type,
                                                          size_bits,
                                                          self_info,
                                                          self,
                                                          *slot
                                                      );
                                                  }
                                              );
                                    if (!result)
                                    {
                                        return result.unwrap_error();
                                    }

                                    // insert to dependency node
                                    self.insert_child(*slot);
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
        namespace argument          = generic_convert_argument;

        a9n::word target_descriptor = 0;
        a9n::word target_depth      = 0;

        return a9n::hal::get_message_register(owner, argument::ROOT_DESCRIPTOR)
            .and_then(
                [&](a9n::word descriptor) -> a9n::hal::hal_result
                {
                    a9n::kernel::utility::logger::printk("generic::configure_registers\n");
                    target_descriptor = descriptor;
                    target_depth      = extract_depth(descriptor);
                    DEBUG_LOG(
                        "descriptor : 0x%016llx   target_depth : "
                        "0x%016llx\n",
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
        generic::calculate_capability_memory_size_bits(capability_type type, a9n::word size_bits)
    {
        DEBUG_LOG("calculate_capability_memory_size_bits : 0x%llx, 0x%llx", type, size_bits);
        switch (type)
        {
            using enum capability_type;

            case NONE :
                return liba9n::option_none;

            case DEBUG :
                return 0;

            case NODE :
                return liba9n::calculate_radix_ceil(
                    liba9n::calculate_radix_ceil(sizeof(capability_node))
                    + liba9n::calculate_radix_ceil(sizeof(capability_slot) * size_bits)
                );

            case GENERIC :
                return size_bits;

            case PAGE_TABLE :
                return liba9n::calculate_radix(a9n::PAGE_SIZE);

            case FRAME :
                return liba9n::calculate_radix(a9n::PAGE_SIZE);

            case PROCESS_CONTROL_BLOCK :
                return liba9n::calculate_radix_ceil(sizeof(process_control_block));

            case IPC_PORT :
            case NOTIFICATION_PORT :
            case INTERRUPT :
            case IO_PORT :
            case VIRTUAL_CPU :
            case VIRTUAL_PAGE_TABLE :
            default :
                return 0;
        }
    }

    capability_result generic::try_make_capability(
        capability_type  type,
        a9n::word        size_bits,
        generic_info    &info,
        capability_slot &self,
        capability_slot &target_slot
    )
    {
        // capability unit size (2^memory_size_bits = real size)
        auto memory_size_bits = calculate_capability_memory_size_bits(type, size_bits).unwrap_or(0);

        switch (type)
        {
            using enum capability_type;

            case NONE :
                a9n::kernel::utility::logger::error("invalid capability type");
                return capability_error::ILLEGAL_OPERATION;

            case DEBUG :
                return {};

            case NODE :
                return {};

            case GENERIC :
                {
                    auto request_unit_size = (static_cast<a9n::word>(1) << memory_size_bits);
                    auto aligned_watermark
                        = liba9n::align_value(info.current_watermark(), request_unit_size);
                    auto child_info = generic_info(
                        aligned_watermark,
                        memory_size_bits,
                        info.is_device(),
                        aligned_watermark
                    );
                    info.apply_allocate(memory_size_bits);

                    // update self
                    return try_configure_generic_slot(self, info)
                        .and_then(
                            [&](void) -> kernel_result
                            {
                                // update child
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

            case PAGE_TABLE :
            case FRAME :
            case PROCESS_CONTROL_BLOCK :
            case IPC_PORT :
            case NOTIFICATION_PORT :
            case INTERRUPT :
            case IO_PORT :
            case VIRTUAL_CPU :
            case VIRTUAL_PAGE_TABLE :
            default :
                break;
        }

        return {};
    }

    capability_result generic::revoke()
    {
        return {};
    }

}
