#ifndef CAPABILITY_TYPE_HPP
#define CAPABILITY_TYPE_HPP

#include <library/common/types.hpp>

namespace library::capability
{
    enum class capability_type : uint16_t
    {
        // memory
        generic,
        frame,

        // process
        domain,
        task_control_block,

        // communication
        ipc_port,
        interrupt,
    };
}

#endif
