#include <kernel/capability/address_space.hpp>

#include <kernel/capability/capability_result.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/memory_manager.hpp>

namespace a9n::kernel
{
    // helper
    namespace
    {
        inline capability_error convert_memory_map_to_capability_error(memory_map_error e)
        {
            switch (e)
            {
                case memory_map_error::ILLEGAL_DEPTH :
                case memory_map_error::INVALID_PAGE_TABLE :
                case memory_map_error::INVALID_FRAME :
                case memory_map_error::NO_SUCH_PAGE_TABLE :
                    return capability_error::INVALID_DESCRIPTOR;

                case memory_map_error::ILLEGAL_AUTORITY :
                    return capability_error::PERMISSION_DENIED;

                case memory_map_error::ALREADY_MAPPED :
                    return capability_error::ILLEGAL_OPERATION;

                default :
                    return capability_error::FATAL;
            }
        };

        inline capability_error
            convert_lookup_to_capability_error([[maybe_unused]] capability_lookup_error e)
        {
            return capability_error::INVALID_DESCRIPTOR;
        };

        inline liba9n::result<a9n::word, capability_error>
            get_message_register(process &owner, a9n::word index)
        {
            return a9n::hal::get_message_register(owner, index)
                .transform_error(
                    [](a9n::hal::hal_error e) -> capability_error
                    {
                        return capability_error::FATAL;
                    }
                );
        }

        inline capability_result validate_root_address_space(const page_table &target_root)
        {
            return a9n::hal::validate_root_address_space(target_root)
                .transform_error(convert_memory_map_to_capability_error);

            return {};
        }
    }

    capability_result address_space::execute(process &owner, capability_slot &self)
    {
        auto target_operation = [&](void) -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (target_operation())
        {
            case operation_type::MAP :
                DEBUG_LOG("address_space::map");
                return operation_map(owner, self);

            case operation_type::UNMAP :
                DEBUG_LOG("address_space::unmap");
                return operation_unmap(owner, self);

            case operation_type::GET_UNSET_DEPTH :
                DEBUG_LOG("address_space::get_unset_depth");
                return operation_get_unset_depth(owner, self);

            default :
                DEBUG_LOG("address_space::<ILLEGAL_OPERATION>");
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result address_space::operation_map(process &owner, capability_slot &self)
    {
        auto root_table = convert_slot_data_to_page_table(self.data);

        return validate_root_address_space(root_table)
            .and_then(
                [&](void) -> liba9n::result<a9n::word, capability_error>
                {
                    return get_message_register(owner, MAP_DESCRIPTOR);
                }
            )
            .and_then(
                [&](a9n::word descriptor) -> capability_result
                {
                    // get root page table from descriptor
                    return owner.root_slot.component
                        ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [&](capability_slot *slot) -> capability_result
                            {
                                auto root_table = convert_slot_data_to_page_table(self.data);

                                switch (slot->type)
                                {
                                    case capability_type::PAGE_TABLE :
                                        {
                                            DEBUG_LOG("address_space::map : PAGE_TABLE");
                                            auto target_table
                                                = convert_slot_data_to_page_table(slot->data);

                                            return get_message_register(owner, MAP_ADDRESS)
                                                .and_then(
                                                    [&](a9n::word address) -> capability_result
                                                    {
                                                        if (!a9n::hal::is_valid_user_address(address))
                                                        {
                                                            return capability_error::INVALID_ARGUMENT;
                                                        }

                                                        DEBUG_LOG(
                                                            "map : [0x%016llx] 0x%016llx -> "
                                                            "0x%016llx",
                                                            root_table.address,
                                                            address,
                                                            target_table.address
                                                        );

                                                        DEBUG_LOG("hal::map_page_table");
                                                        return a9n::hal::map_page_table(
                                                                   root_table,
                                                                   target_table,
                                                                   address
                                                        )
                                                            .transform_error(
                                                                convert_memory_map_to_capability_error
                                                            );
                                                    }
                                                );
                                        }

                                    case capability_type::FRAME :
                                        {
                                            DEBUG_LOG("address_space::map : FRAME");
                                            auto target_frame = convert_slot_data_to_frame(slot->data);

                                            return get_message_register(owner, MAP_ADDRESS)
                                                .and_then(
                                                    [&](a9n::word address) -> capability_result
                                                    {
                                                        if (!a9n::hal::is_valid_user_address(address))
                                                        {
                                                            return capability_error::INVALID_ARGUMENT;
                                                        }

                                                        DEBUG_LOG(
                                                            "map : [0x%016llx] 0x%016llx -> "
                                                            "0x%016llx",
                                                            root_table.address,
                                                            address,
                                                            target_frame.address
                                                        );

                                                        DEBUG_LOG("hal::map_frame");
                                                        return a9n::hal::map_frame(
                                                                   root_table,
                                                                   target_frame,
                                                                   address
                                                        )
                                                            .transform_error(
                                                                convert_memory_map_to_capability_error
                                                            );
                                                    }
                                                );
                                        }

                                    default :
                                        DEBUG_LOG("address_space::map : INVALID_DESCRIPTOR");
                                        return capability_error::INVALID_DESCRIPTOR;
                                }
                            }
                        );
                }
            );
    }

    capability_result address_space::operation_unmap(process &owner, capability_slot &self)
    {
        auto root_table = convert_slot_data_to_page_table(self.data);

        return validate_root_address_space(root_table)
            .and_then(
                [&](void) -> liba9n::result<a9n::word, capability_error>
                {
                    return get_message_register(owner, UNMAP_DESCRIPTOR);
                }
            )
            .and_then(
                [&](a9n::word descriptor) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [&](capability_slot *slot) -> capability_result
                            {
                                switch (slot->type)
                                {
                                    case capability_type::PAGE_TABLE :
                                        {
                                            auto target_table
                                                = convert_slot_data_to_page_table(slot->data);

                                            return get_message_register(owner, UNMAP_ADDRESS)
                                                .and_then(
                                                    [&](a9n::word address) -> capability_result
                                                    {
                                                        if (!a9n::hal::is_valid_user_address(descriptor))
                                                        {
                                                            return capability_error::INVALID_ARGUMENT;
                                                        }

                                                        DEBUG_LOG(
                                                            "unmap : [0x%016llx] 0x%016llx -> "
                                                            "0x%016llx",
                                                            root_table.address,
                                                            address,
                                                            target_table.address
                                                        );

                                                        return a9n::hal::unmap_page_table(
                                                                   root_table,
                                                                   target_table,
                                                                   address
                                                        )
                                                            .transform_error(
                                                                convert_memory_map_to_capability_error
                                                            );
                                                    }
                                                );
                                        }

                                    case capability_type::FRAME :
                                        {
                                            auto target_frame = convert_slot_data_to_frame(slot->data);

                                            return get_message_register(owner, UNMAP_ADDRESS)
                                                .and_then(
                                                    [&](a9n::word address) -> capability_result
                                                    {
                                                        if (!a9n::hal::is_valid_user_address(address))
                                                        {
                                                            return capability_error::INVALID_ARGUMENT;
                                                        }

                                                        DEBUG_LOG(
                                                            "unmap : [0x%016llx] 0x%016llx -> "
                                                            "0x%016llx",
                                                            root_table.address,
                                                            address,
                                                            target_frame.address
                                                        );

                                                        return a9n::hal::unmap_frame(
                                                                   root_table,
                                                                   target_frame,
                                                                   address
                                                        )
                                                            .transform_error(
                                                                convert_memory_map_to_capability_error
                                                            );
                                                    }
                                                );
                                        }

                                    default :
                                        return capability_error::INVALID_DESCRIPTOR;
                                }
                            }
                        );
                }
            );
    }

    capability_result address_space::operation_get_unset_depth(process &owner, capability_slot &self)
    {
        auto self_table = convert_slot_data_to_page_table(self.data);

        auto get_unset_depth_argument_address =
            [&](void) -> liba9n::result<a9n::virtual_address, capability_error>
        {
            return get_message_register(owner, GET_UNSET_DEPTH_ADDRESS);
        };

        auto get_unset_depth =
            [&](a9n::virtual_address address) -> liba9n::result<a9n::word, capability_error>
        {
            return a9n::hal::search_unset_page_table_depth(self_table, address)
                .transform_error(convert_memory_map_to_capability_error);
        };

        auto write_result = [&](a9n::word depth) -> capability_result
        {
            return a9n::hal::configure_message_register(owner, RESULT_DEPTH, depth)
                .transform_error(convert_hal_to_kernel_error)
                .transform_error(convert_kernel_to_capability_error);
        };

        return validate_root_address_space(self_table)
            .and_then(get_unset_depth_argument_address)
            .and_then(get_unset_depth)
            .and_then(write_result);
    }

}
