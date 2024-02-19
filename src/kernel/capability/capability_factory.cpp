#include <kernel/capability/capability_factory.hpp>

#include <library/capability/capability_types.hpp>
#include <library/common/calculate.hpp>

// node components
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/capability_entry.hpp>

namespace kernel
{
    common::word capability_factory::calculate_real_size(
        library::capability::capability_type type,
        common::word size
    )
    {
        switch (type)
        {
            case library::capability::capability_type::NODE :
                {
                    return calculate_capability_node_size(size);
                }

            default :
                {
                    return 0;
                }
        }
    }

    common::word
        capability_factory::calculate_capability_node_size(common::word size)
    {
        constexpr auto node_size = sizeof(capability_node);
        constexpr auto entry_size = sizeof(capability_entry);

        const auto aligned_node_size
            = library::common::round_up_power_of_two(node_size);

        const auto aligned_entry_size
            = library::common::round_up_power_of_two(entry_size);

        const auto all_entry_size = node_size + (entry_size * size);

        const auto aligned_all_entry_size
            = library::common::round_up_power_of_two(all_entry_size);
    }

}
