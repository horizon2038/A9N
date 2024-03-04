#include "library/common/types.hpp"
#include <kernel/capability/capability_factory.hpp>

#include <kernel/capability/generic.hpp>

#include <kernel/utility/logger.hpp>
#include <library/capability/capability_types.hpp>

namespace kernel
{
    common::word capability_factory::calculate_memory_size(
        common::word type,
        common::word size_flags
    )
    {
        using namespace library::capability;

        auto target_type = static_cast<capability_type>(type);

        switch (target_type)
        {
            case capability_type::GENERIC :
                {
                    return static_cast<common::word>(1) << size_flags;
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
        common::word type,
        common::word size_flags,
        common::virtual_address target_address
    )
    {
        using namespace library::capability;

        capability_slot slot;

        auto target_type = static_cast<capability_type>(type);

        switch (target_type)
        {
            case capability_type::GENERIC :
                {
                    // generic ptr
                    slot.data.set_element(0, target_address);
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
