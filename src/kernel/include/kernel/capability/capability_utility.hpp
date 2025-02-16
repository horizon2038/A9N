#ifndef A9N_KERNEL_CAPABILITY_UTILITY_HPP
#define A9N_KERNEL_CAPABILITY_UTILITY_HPP

#include <hal/interface/process_manager.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/types.hpp>
#include <liba9n/common/not_null.hpp>

// common utility functions for capability management

namespace a9n::kernel
{
    inline constexpr liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
        try_get_slot_from_node_and_index(
            const process        &owner,
            capability_descriptor destination_node,
            a9n::word             destination_node_index
        )
    {
        if ((owner.root_slot.type != capability_type::NODE) || !(owner.root_slot.component))
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        return owner.root_slot.component
            ->traverse_slot(destination_node, extract_depth(destination_node), a9n::BYTE_BITS)
            .and_then(
                [&](capability_slot *slot
                ) -> liba9n::result<liba9n::not_null<capability_slot>, capability_lookup_error>
                {
                    return slot->component->retrieve_slot(destination_node_index)
                        .and_then(
                            [&](capability_slot *slot
                            ) -> liba9n::result<liba9n::not_null<capability_slot>, capability_lookup_error>
                            {
                                return *slot;
                            }
                        );
                }
            )
            .transform_error(
                [](capability_lookup_error e) -> kernel_error
                {
                    return kernel_error::ILLEGAL_ARGUMENT;
                }
            );
    };

    inline constexpr liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
        try_get_slot_from_node_and_index_in_message(
            const process &owner,
            a9n::word      destination_node_descriptor_offset, // node descriptor offset in MR
            a9n::word      destination_node_index_offset       // node index offset in MR
        )
    {
        auto get_message = [&](a9n::word offset) -> decltype(auto)
        {
            return a9n::hal::get_message_register(owner, offset)
                .transform_error(convert_hal_to_kernel_error);
        };

        return get_message(destination_node_descriptor_offset)
            .and_then(
                [&](a9n::word node_descriptor
                ) -> liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
                {
                    return get_message(destination_node_index_offset)
                        .and_then(
                            [&](a9n::word index
                            ) -> liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
                            {
                                return try_get_slot_from_node_and_index(owner, node_descriptor, index);
                            }
                        );
                }
            );
    }

}

#endif
