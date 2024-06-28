#ifndef IPC_PORT_HPP
#define IPC_PORT_HPP

#include <kernel/capability/capability_component.hpp>

namespace a9n::kernel
{
    class ipc_port : public capability_component
    {
      public:
        a9n::error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer  *buffer
        ) override;

        a9n::error revoke() override
        {
            return 0;
        }

        capability_slot *retrieve_slot(a9n::word index) override
        {
            return nullptr;
        }

        capability_slot *traverse_slot(
            liba9n::capability::capability_descriptor descriptor,
            a9n::word                                 max_bits,
            a9n::word                                 used_bits
        ) override
        {
            return nullptr;
        };

      private:
        a9n::error decode_operation(message_buffer *buffer);
    };
}

#endif
