#include "kernel/capability/capability_local_state.hpp"
#include <kernel/capability/generic.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    generic::generic(
        common::physical_address initial_start_address,
        common::word initial_size,
        bool initial_flags
    )
        : start_address(initial_start_address)
        , size(initial_size)
        , flags(initial_flags)
        , watermark(0)
    {
    }

    common::error generic::execute(
        message_buffer *buffer,
        capability_local_state *local_state
    )
    {
        return 0;
    }

    common::error generic::revoke()
    {
        return 0;
    }

    // generic::convert is a kind of "factory pattern".
    common::error generic::convert()
    {
        common::word type = 0;
        switch (type)
        {
            case 0 :
                break;

            default :
                break;
        }
        return 0;
    }

}
