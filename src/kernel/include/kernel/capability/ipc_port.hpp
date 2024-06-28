#ifndef IPC_PORT_HPP
#define IPC_PORT_HPP

#include <kernel/capability/capability_component.hpp>

namespace kernel
{
    class ipc_port : public capability_component
    {
      public:
        common::error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer  *buffer
        ) override;

        common::error revoke() override
        {
            return 0;
        }

        capability_slot *retrieve_slot(common::word index) override
        {
            return nullptr;
        }

        capability_slot *traverse_slot(
            library::capability::capability_descriptor descriptor,
            common::word                               max_bits,
            common::word                               used_bits
        ) override
        {
            return nullptr;
        };

      private:
        common::error decode_operation(message_buffer *buffer);
    };
}

#endif
