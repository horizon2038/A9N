#include "library/common/types.hpp"
#include <kernel/capability/capability_factory.hpp>

#include <kernel/capability/generic.hpp>

#include <kernel/utility/logger.hpp>
#include <library/capability/capability_types.hpp>

namespace kernel
{
    common::word capability_factory::calculate_memory_size_bits(
        library::capability::capability_type type,
        common::word size_bits
    )
    {
        using namespace library::capability;

        switch (type)
        {
            case capability_type::GENERIC :
                {
                    return size_bits;
                }

            case capability_type::FRAME :
                {
                    return library::common::PAGE_SIZE;
                }

            default :
                {
                    kernel::utility::logger::debug("illegal type");
                    return 0;
                }
        }
    }

    capability_slot capability_factory::make(
        library::capability::capability_type type,
        common::word size_bits,
        common::virtual_address target_address
    )
    {
        using capability_type = library::capability::capability_type;

        capability_slot slot;

        switch (type)
        {
            case capability_type::GENERIC :
                {
                    // generic ptr
                    slot.data.set_element(0, target_address);
                    slot.data.set_element(
                        1,
                        serialize_generic_flags(false, size_bits)
                    );
                    slot.data.set_element(2, target_address);
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
