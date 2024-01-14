#include "library/capability/capability_descriptor.hpp"
#include "library/common/types.hpp"
#include <kernel/capability/capability_node.hpp>
#include <kernel/utility/logger.hpp>

namespace kernel
{
    capability_node::capability_node(
        common::word initial_ignore_bits,
        common::word initial_radix_bits,
        capability_entry *initial_capability_slots
    )
        : capability_slots(initial_capability_slots)
        , ignore_bits(initial_ignore_bits)
        , radix_bits(initial_radix_bits)
    {
    }

    capability_type capability_node::type()
    {
        return capability_type::NODE;
    }

    common::error capability_node::capability_node::execute(capability_data data
    )
    {
        // 1. decode operation
        // 2. dispatch operation
        // 3.run operation
        return 1;
    };

    capability_entry *capability_node::traverse_entry(
        library::capability::capability_descriptor descriptor,
        common::word depth
    )
    {
        if (descriptor == 0)
        {
            return nullptr;
        }

        if (depth == library::common::WORD_BITS)
        {
            return lookup_entry(descriptor, depth);
        }

        auto new_depth = calculate_depth(depth);
        return lookup_entry(descriptor, new_depth);
    }

    // traverse nodes across multiple depths.
    /*
    capability_entry *capability_node::traverse_capability(
        library::capability::capability_descriptor descriptor
    )
    {
        kernel::utility::logger::printk("traverse_capability\n");
        kernel::utility::logger::printk(
            "descriptor\e[55G : 0x%016llx\n",
            descriptor
        );
        auto depth = 0;
        capability *target_capability = this;
        capability_entry *entry;

        auto i = 0;
        while (1)
        {
            kernel::utility::logger::printk("traverse_loop [%02d]\n", i);
            auto is_node = target_capability->type() == capability_type::NODE;
            auto is_depth_remain = ((depth < library::common::WORD_BITS));

            if (!is_depth_remain)
            {
                kernel::utility::logger::printk("traverse succeed\n");
                return entry;
            }

            if (!is_node)
            {
                // invalid depth
                kernel::utility::logger::error("invalid depth !\n");
                return nullptr;
            }

            auto node = static_cast<capability_node *>(target_capability);
            auto lookup_result = node->lookup_entry(descriptor, depth);

            entry = lookup_result.entry;
            depth = lookup_result.depth_bits;

            kernel::utility::logger::printk(
                "capability_entry\e[55G : 0x%016llx\n",
                reinterpret_cast<common::word>(entry)
            );

            if (depth > library::common::WORD_BITS)
            {
                return nullptr;
            }
            target_capability = lookup_result.entry->capability_pointer;
            i++;
        }

        // unreachable
        return nullptr;
    }
    */

    capability_entry *capability_node::lookup_entry(
        library::capability::capability_descriptor descriptor,
        common::word depth_bits
    )
    {
        kernel::utility::logger::printk("lookup_capability\n");
        auto index = calculate_capability_index(descriptor, depth_bits);
        auto entry = index_to_capability_entry(index);
        if (entry->capability_pointer == nullptr)
        {
            kernel::utility::logger::error("null entry !\n");
        }
        return entry;
    }

    capability_entry *
        capability_node::index_to_capability_entry(common::word index)
    {
        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            kernel::utility::logger::error("index out of range !\n");
            return nullptr;
        }
        return &capability_slots[index];
    }
}
