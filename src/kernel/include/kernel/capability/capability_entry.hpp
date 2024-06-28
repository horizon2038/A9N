#ifndef CAPABILITY_ENTRY_HPP
#define CAPABILITY_ENTRY_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_object.hpp>
#include <kernel/capability/capability_local_state.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    struct capability_entry final : public capability_component
    {
      public:
        capability_object     *capability_pointer;
        capability_local_state state;

        a9n::error execute(message_buffer *buffer) override
        {
            a9n::kernel::utility::logger::printk("execute : entry\n");
            for (auto i = 0; i < ENTRY_DATA_MAX; i++)
            {
                a9n::kernel::utility::logger::printk(
                    "entry_data [%02d]\e[55G : 0x%016llx\n",
                    i,
                    state.data.get_element(i)
                );
            }
            if (capability_pointer == nullptr)
            {
                return -1;
            }
            capability_pointer->execute(buffer, &state);
            return 0;
            // return capability_pointer->execute(buffer, &this->state);
        }

        a9n::error add_child(
            a9n::word          index,
            capability_component *component
        ) override
        {
            return 0;
        };

        capability_component *retrieve_child(a9n::word index) override
        {
            return this;
        };

        a9n::error revoke_child(a9n::word index) override
        {
            return 0;
        }

        a9n::error remove_child(a9n::word index) override
        {
            return 0;
        }

        a9n::error revoke() override
        {
            /*
            capability_pointer = nullptr;
            state.data.fill(0);
            */

            // root
            if (this->state.family_node.preview_capability_entry == nullptr)
            {
                auto next_capability_entry
                    = this->state.family_node.next_capability_entry;
                next_capability_entry->state.family_node.preview_capability_entry
                    = nullptr;
                return 0;
            }

            auto preview_entry
                = this->state.family_node.preview_capability_entry;
            auto next_entry = this->state.family_node.next_capability_entry;

            if (preview_entry != nullptr
                && preview_entry->state.family_node.depth
                       < this->state.family_node.depth)
            {
                preview_entry->state.family_node.next_capability_entry
                    = next_entry;
                if (next_entry != nullptr)
                {
                    next_entry->state.family_node.preview_capability_entry
                        = preview_entry;
                }
            }
            else
            {
                if (preview_entry != nullptr)
                {
                    next_entry->state.family_node.preview_capability_entry
                        = preview_entry;
                }
            }

            return 0;
        }

        // all child nodes are also revoked.
        a9n::error remove() override
        {
            auto current_entry = this->state.family_node.next_capability_entry;
            while (current_entry != nullptr
                   && current_entry->state.family_node.depth
                          > this->state.family_node.depth)
            {
                current_entry->revoke();
                current_entry = this->state.family_node.next_capability_entry;
            }
            return 0;
        }

        capability_component *traverse(
            liba9n::capability::capability_descriptor descriptor,
            a9n::word                               descriptor_max_bits,
            a9n::word                               descriptor_used_bits
        ) override
        {
            return this;
        }
    };
}

#endif
