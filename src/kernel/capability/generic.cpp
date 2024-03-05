#include <kernel/capability/generic.hpp>

#include <kernel/ipc/message_buffer.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_factory.hpp>

#include <library/capability/generic_operation.hpp>
#include <library/capability/capability_types.hpp>
#include <library/common/types.hpp>
#include <library/common/calculate.hpp>

#include <kernel/utility/logger.hpp>

namespace kernel
{
    common::error generic::execute(
        capability_slot *this_slot,
        capability_slot *root_slot,
        message_buffer *buffer
    )
    {
        auto e = decode_operation(this_slot, root_slot, buffer);
        return e;
    }

    common::error generic::decode_operation(
        capability_slot *this_slot,
        capability_slot *root_slot,
        message_buffer *buffer
    )
    {
        kernel::utility::logger::printk("generic : decode_operation\n");
        auto operation_type
            = static_cast<library::capability::generic_operation>(
                buffer->get_element(2)
            );

        switch (operation_type)
        {
            case library::capability::generic_operation::CONVERT :
                {
                    kernel::utility::logger::printk("generic : CONVERT\n");
                    auto e = convert(this_slot, root_slot, buffer);
                    return e;
                }

            default :
                {
                    kernel::utility::logger::printk("illegal operaton\n");
                    return -1;
                }
        }

        return 0;
    }

    // generic::convert is a kind of "factory pattern".
    common::error generic::convert(
        capability_slot *this_slot,
        capability_slot *root_slot,
        message_buffer *buffer
    )
    {
        using namespace library::capability::convert_argument;

        auto this_info = create_generic_info(&(this_slot->data));
        capability_factory factory {};
        auto target_type = buffer->get_element(CAPABILITY_TYPE);
        auto size_bits = buffer->get_element(CAPABILITY_SIZE_BITS);
        auto size = factory.calculate_memory_size(target_type, size_bits);

        return 0;
    }

    common::error generic::create_generic(
        capability_slot *this_slot,
        capability_slot *root_slot,
        message_buffer *buffer
    )
    {
        auto this_info = create_generic_info(&(this_slot->data));
        auto child_info = create_child_generic_info(&this_info, buffer);

        if (!is_valid_size(&this_info, &child_info))
        {
            // invalid size
            kernel::utility::logger::debug("invalid size!");
            return -1;
        }

        auto new_info = update_parent_generic_info(&this_info, &child_info);
        this_slot->data = create_generic_slot_data(&new_info);

        auto child_slot = retrieve_target_slot(root_slot, buffer);

        if (child_slot == nullptr)
        {
            // null slot
            return -1;
        }

        child_slot->data = create_generic_slot_data(&child_info);

        return 0;
    };

    generic_info generic::create_generic_info(capability_slot_data *data)
    {
        return generic_info {
            .base_address = data->get_element(0),
            .flags = generic_flags(data->get_element(1)),
            .watermark = data->get_element(2),
        };
    }

    /*
    generic_info generic::create_child_generic_info(
        generic_info *parent_info,
        message_buffer *buffer
    )
    {
        using namespace library::capability::convert_argument;

        generic_info info;

        auto target_size_bits = calculate_generic_size_bits(
            buffer->get_element(CAPABILITY_SIZE_BITS)
        );

        auto target_size = static_cast<common::word>(1) << target_size_bits;
        auto aligned_target_address
            = library::common::align_value(parent_info->watermark, target_size);

        info.base_address = aligned_target_address;
        info.size_bits = target_size_bits;
        info.is_device = 0;
        info.watermark = aligned_target_address;

        return info;
    }
    */

    capability_slot *generic::retrieve_target_slot(
        capability_slot *root_slot,
        message_buffer *buffer
    )
    {
        using namespace library::capability::convert_argument;

        auto target_descriptor = buffer->get_element(ROOT_DESCRIPTOR);
        auto target_depth = buffer->get_element(ROOT_DEPTH);
        auto target_index = buffer->get_element(SLOT_INDEX);

        if (target_depth == 0)
        {
            return root_slot->component->retrieve_slot(target_index);
        }

        auto target_slot = root_slot->component->traverse_slot(
            target_descriptor,
            target_depth,
            0
        );

        return target_slot->component->retrieve_slot(target_index);
    }

    bool generic::is_allocatable(generic_info *parent_info, common::word size)
    {
        auto parent_size = parent_info->flags.size();
        return (
            (parent_info->base_address + parent_size)
            > (child_info->base_address + child_size)
        );
    }

    /*
    generic_info generic::update_parent_generic_info(
        generic_info *parent_info,
        generic_info *child_info
    )
    {
        generic_info info;

        auto child_size = static_cast<common::word>(1) << child_info->size_bits;

        info.base_address = parent_info->base_address;
        info.size_bits = parent_info->size_bits;
        info.is_device = parent_info->is_device;
        info.watermark = child_info->base_address + child_size;

        return info;
    }

    capability_slot_data generic::create_generic_slot_data(generic_info *info)
    {
        capability_slot_data data;

        auto flags = create_generic_flags(info->is_device, info->size_bits);

        data.set_element(0, info->base_address);
        data.set_element(1, flags.value);
        data.set_element(2, info->watermark);

        return data;
    }
    */

    common::error generic::revoke()
    {
        return 0;
    }

}
