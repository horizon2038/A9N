#ifndef A9N_KERNEL_ADDRESS_SPACE_HPP
#define A9N_KERNEL_ADDRESS_SPACE_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/page_table_capability.hpp>
#include <kernel/memory/memory_type.hpp>
#include <kernel/types.hpp>

#include <hal/interface/memory_manager.hpp>

namespace a9n::kernel
{
    class address_space : public capability_component
    {
      private:
        enum operation_index : a9n::word
        {
            RESERVED = 0, // descriptor
            OPERATION_TYPE,

            RESERVED_0,
            RESERVED_1,
            RESERVED_2,

            // for map
            MAP_DESCRIPTOR = RESERVED_0,
            MAP_ADDRESS    = RESERVED_1,
            ATTRIBUTE      = RESERVED_2,

            // for unmap
            UNMAP_DESCRIPTOR = RESERVED_0,
            UNMAP_ADDRESS    = RESERVED_1,

            // for get_unset_depth
            GET_UNSET_DEPTH_ADDRESS = RESERVED_0,
        };

        enum result_index : a9n::word
        {
            IS_SUCCESS,
            ERROR_CODE,

            // for get_address
            RESULT_DEPTH = RESERVED_0,
        };

        enum operation_type : a9n::word
        {
            NONE,
            MAP,
            UNMAP,
            GET_UNSET_DEPTH
        };

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_map(process &owner, capability_slot &self);
        capability_result operation_unmap(process &owner, capability_slot &self);
        capability_result operation_get_unset_depth(process &owner, capability_slot &self);

        capability_result revoke(capability_slot &self) override
        {
            return {};
        }

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

    // because it is a "processor", the component is a single instance;
    // uniqueness is determined by capability_slot_data.
    inline address_space address_space_core;

    inline kernel_result
        try_configure_address_space_slot(capability_slot &slot, const page_table &table)
    {
        return a9n::hal::validate_root_address_space(table)
            .transform_error(
                [](memory_map_error e) -> kernel_error
                {
                    return kernel_error::NO_SUCH_ADDRESS;
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    if (!table.address)
                    {
                        return kernel_error::ILLEGAL_ARGUMENT;
                    }

                    slot.rights    = capability_slot::object_rights::ALL;
                    slot.component = &address_space_core;
                    slot.type      = capability_type::ADDRESS_SPACE;
                    slot.data      = convert_page_table_to_slot_data(table);

                    return {};
                }
            );
    }

}

#endif
