#include <kernel/types.hpp>

#include <kernel/capability/capability_factory.hpp>

#include <kernel/capability/generic.hpp>

#include <kernel/utility/logger.hpp>
#include <liba9n/capability/capability_types.hpp>

namespace a9n::kernel
{
    a9n::word capability_factory::calculate_memory_size_bits(
        liba9n::capability::capability_type type,
        a9n::word                           size_bits
    )
    {
        using namespace liba9n::capability;

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
        liba9n::capability::capability_type type,
        a9n::word                           size_bits,
        a9n::virtual_address                target_address
    )
    {
        using capability_type = liba9n::capability::capability_type;

        capability_slot slot;

        switch (type)
        {
            case capability_type::GENERIC :
                {
                    // generic ptr
                    slot.set_local_data<0>(target_address);
                    slot.set_local_data<1>(serialize_generic_flags(false, size_bits)
                    );
                    slot.set_local_data<2>(target_address);
                    break;
                }

            default :
                {
                    break;
                }
        }

        return slot;
    }
}
