#ifndef IPC_PORT_HPP
#define IPC_PORT_HPP

#include <kernel/capability/capability_component.hpp>

namespace a9n::kernel
{
    class ipc_port : public capability_component
    {
      public:
        capability_error execute(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        ) override;

        capability_error revoke() override
        {
            return {};
        }

        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            return capability_lookup_error::TERMINAL;
        }

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  max_bits,
            a9n::word                  used_bits
        ) override
        {
            return capability_lookup_error::TERMINAL;
        };

      private:
        a9n::error decode_operation(ipc_buffer *buffer);
    };
}

#endif
