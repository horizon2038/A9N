#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/ipc_port.hpp>

namespace a9n::kernel
{
    capability_error ipc_port::execute(
        capability_slot *this_slot,
        capability_slot *root_slot,
        ipc_buffer      *buffer
    )
    {
    }

    a9n::error decode_operation(ipc_buffer *buffer)
    {
    }

}
