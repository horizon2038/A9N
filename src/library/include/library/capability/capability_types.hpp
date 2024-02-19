#ifndef CAPABILITY_TYPE_HPP
#define CAPABILITY_TYPE_HPP

#include <library/common/types.hpp>

namespace library::capability
{
    enum class capability_type : uint16_t
    {
        NODE,

        // memory
        GENERIC,
        PAGE_TABLE,
        FRAME,

        // process
        DOMAIN,
        PROCESS_CONTROL_BLOCK,

        // communication
        IPC_PORT,
        INTERRUPT,
    };
}

#endif
