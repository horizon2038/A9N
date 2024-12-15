#include <kernel/types.hpp>

#include <kernel/capability/capability_factory.hpp>

#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/capability/generic.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    a9n::word capability_factory::calculate_memory_size_bits(capability_type type, a9n::word size_bits)
    {
        switch (type)
        {
            case capability_type::GENERIC :
                {
                    return size_bits;
                }

            case capability_type::FRAME :
                {
                    return a9n::PAGE_SIZE;
                }

            default :
                {
                    a9n::kernel::utility::logger::debug("illegal type");
                    return 0;
                }
        }
    }

    capability_slot capability_factory::make(
        capability_type      type,
        a9n::word            size_bits,
        a9n::virtual_address target_address
    )
    {
        capability_slot slot;

        switch (type)
        {
            case capability_type::GENERIC :
                {
                    // generic ptr
                    slot.set_local_data<0>(target_address);
                    slot.set_local_data<1>(serialize_generic_flags(false, size_bits));
                    slot.set_local_data<2>(target_address);
                    break;
                }

            case capability_type::NODE :

            default :
                {
                    break;
                }
        }

        return slot;
    }
}
