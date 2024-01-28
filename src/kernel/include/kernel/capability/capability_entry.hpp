#ifndef CAPABILITY_ENTRY_HPP
#define CAPABILITY_ENTRY_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_object.hpp>
#include <kernel/capability/capability_local_state.hpp>

#include <kernel/utility/logger.hpp>

namespace kernel
{
    struct capability_entry final : public capability_component
    {
      public:
        capability_object *capability_pointer;
        capability_local_state state;

        common::error execute(message_buffer *buffer) override
        {
            kernel::utility::logger::printk("execute : entry\n");
            for (auto i = 0; i < ENTRY_DATA_MAX; i++)
            {
                kernel::utility::logger::printk(
                    "entry_data [%02d]\e[55G : 0x%016llx\n",
                    i,
                    state.data.get_element(i)
                );
            }
            return 0;
            // return capability_pointer->execute(buffer, &this->state);
        }

        common::error revoke() override
        {
            capability_pointer = nullptr;
            state.data.fill(0);
            return 0;
        }

        capability_component *traverse(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        ) override
        {
            return this;
        }

        // all child nodes are also revoked.
        common::error revoke_all()
        {
            return 0;
        }
    };

}

#endif
