#ifndef IPC_PORT_HPP
#define IPC_PORT_HPP

#include <kernel/capability/capability_component.hpp>

namespace a9n::kernel
{
    class ipc_port : public capability_component
    {
      public:
        capability_error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            ipc_buffer      *buffer
        ) override;

        capability_error revoke() override
        {
            return {};
        }

        capability_slot *retrieve_slot(a9n::word index) override
        {
            return nullptr;
        }

        capability_slot *traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  max_bits,
            a9n::word                  used_bits
        ) override
        {
            return nullptr;
        };

      private:
        a9n::error decode_operation(ipc_buffer *buffer);
    };
}

#endif
