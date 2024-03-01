#include "kernel/ipc/message_buffer.hpp"
#include <kernel/capability/generic.hpp>

#include <kernel/capability/capability_local_state.hpp>

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
        auto target_type = static_cast<library::capability::capability_type>(
            buffer->get_element(3)
        );
        switch (target_type)
        {
            case library::capability::capability_type::GENERIC :
                {
                    kernel::utility::logger::printk(
                        "generic.convert -> generic\n"
                    );
                    auto e = create_generic(this_slot, root_slot, buffer);
                    return e;
                }

            case library::capability::capability_type::PAGE_TABLE :
                kernel::utility::logger::printk("generic.convert -> generic\n");
                break;

            case library::capability::capability_type::FRAME :
                kernel::utility::logger::printk("generic.convert -> generic\n");
                break;

            default :
                kernel::utility::logger::printk("illegal type\n");
                break;
        }
        return 0;
    }

    common::error generic::create_generic(
        capability_slot *this_slot,
        capability_slot *root_slot,
        message_buffer *buffer
    )
    {
        auto this_base_address = this_slot->data.get_element(0);
        auto this_size = calculate_size(this_slot->data.get_element(1));
        auto this_watermark = this_slot->data.get_element(2);
        kernel::utility::logger::debug("watermark : %llu\n", this_watermark);

        auto target_size
            = (static_cast<common::word>(1) << buffer->get_element(4));
        kernel::utility::logger::debug("target_size : %llu\n", target_size);
        auto target_count = buffer->get_element(5);
        auto target_descriptor = buffer->get_element(6);
        auto target_depth = buffer->get_element(7);
        auto target_index = buffer->get_element(8);

        auto aligned_target_address
            = library::common::align_value(this_watermark, target_size);

        kernel::utility::logger::debug(
            "aligned_target_address : %llu\n",
            aligned_target_address
        );

        kernel::utility::logger::debug(
            "this_base_address : %llu\n",
            this_base_address
        );
        kernel::utility::logger::debug("this_size : %llu\n", this_size);

        if ((aligned_target_address + target_size)
            > this_base_address + this_size)
        {
            kernel::utility::logger::debug("free space does not exist\n");
            return -1;
        }

        // auto target_slot = root_slot->component->retrieve_slot(target_index);

        capability_slot *target_node_slot;
        if (target_depth == 0)
        {
            target_node_slot = root_slot;
        }
        else
        {
            auto target_node_slot = root_slot->component->traverse_slot(
                target_descriptor,
                target_depth,
                0
            );
        }

        kernel::utility::logger::debug(
            "index : %llu\n",
            buffer->get_element(6)
        );
        kernel::utility::logger::debug(
            "max_bits : %llu\n",
            buffer->get_element(7)
        );

        this_slot->data.set_element(2, aligned_target_address + target_size);

        auto target_slot
            = target_node_slot->component->retrieve_slot(target_index);

        if (target_slot == nullptr)
        {
            kernel::utility::logger::debug("target slot is nullptr\n");
            return -1;
        }

        target_slot->data.set_element(0, aligned_target_address);
        target_slot->data.set_element(1, buffer->get_element(4));
        target_slot->data.set_element(2, aligned_target_address);
        return 0;
    };

    common::error generic::revoke()
    {
        return 0;
    }

}
