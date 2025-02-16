#include <kernel/capability/io_port_capability.hpp>

#include <kernel/capability/capability_utility.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/utility/logger.hpp>
#include <liba9n/common/not_null.hpp>

namespace a9n::kernel
{
    // utility functions
    namespace
    {
        capability_result
            judge_range_is_subset(io_port_address_range &current_range, io_port_address_range &new_range)
        {
            if (new_range.min > new_range.max)
            {
                return capability_error::INVALID_ARGUMENT;
            }

            if ((new_range.min < current_range.min || new_range.max > current_range.max))
            {
                return capability_error::PERMISSION_DENIED;
            }

            return {};
        }

        bool is_address_is_in_range(io_port_address_range &range, a9n::word address)
        {
            return (address >= range.min && address <= range.max);
        }

    }

    capability_result io_port_capability::execute(process &owner, capability_slot &self)
    {
        auto target_operation = [&](void) -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (target_operation())
        {
            case operation_type::OPERATION_READ :
                DEBUG_LOG("operation : read");
                return operation_read(owner, self);

            case operation_type::OPERATION_WRITE :
                DEBUG_LOG("operation : write");
                return operation_write(owner, self);

            case operation_type::OPERATION_MINT :
                DEBUG_LOG("operation : mint");
                return operation_mint(owner, self);

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result io_port_capability::operation_read(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ))
        {
            DEBUG_LOG("permission denied");
            return capability_error::PERMISSION_DENIED;
        }

        // TODO: check address range

        return a9n::hal::get_message_register(owner, SOURCE_PORT)
            .and_then(a9n::hal::read_io_port)
            .transform_error(convert_hal_to_capability_error)
            .and_then(
                [&](a9n::word data) -> capability_result
                {
                    DEBUG_LOG("read 0x%16llx", data);
                    return a9n::hal::configure_message_register(owner, READ_DATA, data)
                        .transform_error(convert_hal_to_capability_error);
                }
            );
    }

    capability_result io_port_capability::operation_write(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::WRITE))
        {
            DEBUG_LOG("permission denied");
            return capability_error::PERMISSION_DENIED;
        }

        // TODO: check address range

        return a9n::hal::get_message_register(owner, DESTINATION_PORT)
            .transform_error(convert_hal_to_capability_error)
            .and_then(
                [&](a9n::word port) -> capability_result
                {
                    auto self_range = convert_slot_data_to_address_range(self.data);
                    if (!is_address_is_in_range(self_range, port))
                    {
                        return capability_error::PERMISSION_DENIED;
                    }

                    return a9n::hal::get_message_register(owner, WRITE_DATA)
                        .and_then(
                            [&](a9n::word data) -> hal::hal_result
                            {
                                DEBUG_LOG("write 0x%16llx -> 0x%016llx", data, port);
                                return a9n::hal::write_io_port(port, data);
                            }
                        )
                        .transform_error(convert_hal_to_capability_error);
                }
            );
    }

    capability_result io_port_capability::operation_mint(process &owner, capability_slot &self)
    {
        auto get_address_range = [&](void) -> io_port_address_range
        {
            auto range_min = a9n::hal::get_message_register(owner, NEW_ADDRESS_MIN)
                                 .unwrap_or(static_cast<a9n::word>(0));
            auto range_max = a9n::hal::get_message_register(owner, NEW_ADDRESS_MAX)
                                 .unwrap_or(static_cast<a9n::word>(0));

            return io_port_address_range { .min = range_min, .max = range_max };
        };

        auto self_range   = convert_slot_data_to_address_range(self.data);
        auto new_range    = get_address_range();

        auto judge_result = judge_range_is_subset(self_range, new_range);
        if (!judge_result)
        {
            return judge_result;
        }

        return try_get_slot_from_node_and_index_in_message(owner, DESTINATION_NODE, DESTINATION_NODE_INDEX)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](liba9n::not_null<capability_slot> destination_slot) -> capability_result
                {
                    if (destination_slot->type != capability_type::NONE)
                    {
                        return capability_error::INVALID_ARGUMENT;
                    }

                    return try_configure_io_port_slot(destination_slot.get(), *this, new_range)
                        .transform_error(convert_kernel_to_capability_error);

                    return {};
                }
            );

        return {};
    }
}
