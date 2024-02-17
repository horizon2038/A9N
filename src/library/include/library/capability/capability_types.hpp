#ifndef CAPABILITY_TYPE_HPP
#define CAPABILITY_TYPE_HPP

#include <library/common/types.hpp>

namespace library::capability
{
    enum class capability_type : uint16_t
    {
        // memory
        generic,
        page_table,
        frame,

        // process
        domain,
        process_control_block,

        // communication
        ipc_port,
        interrupt,
    };
}

#endif
