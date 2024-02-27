#include "kernel/ipc/message_buffer.hpp"
#include <kernel/capability/generic.hpp>

#include <kernel/capability/capability_local_state.hpp>

#include <library/capability/generic_operation.hpp>
#include <library/capability/capability_types.hpp>
#include <library/common/types.hpp>

#include <kernel/utility/logger.hpp>

namespace kernel
{
    common::error
        generic::execute(capability_slot *this_slot, message_buffer *buffer)
    {
        auto e = decode_operation(buffer, &(this_slot->state));
        return e;
    }

    common::error generic::decode_operation(
        message_buffer *buffer,
        capability_local_state *state
    )
    {
        auto operation_type
            = static_cast<library::capability::generic_operation>(
                buffer->get_element(2)
            );

        switch (operation_type)
        {
            case library::capability::generic_operation::CONVERT :
                {
                    kernel::utility::logger::printk("generic : CONVERT\n");
                    convert(buffer, state);
                    break;
                }

            default :
                {
                    kernel::utility::logger::printk("illegal operaton\n");
                    return -1;
                }
        }

        return 0;
    }

    // generic::convert is a kind of "factory pattern".
    common::error
        generic::convert(message_buffer *buffer, capability_local_state *state)
    {
        auto target_type = static_cast<library::capability::capability_type>(
            buffer->get_element(3)
        );
        switch (target_type)
        {
            case library::capability::capability_type::GENERIC :
                kernel::utility::logger::printk("generic.convert -> generic\n");
                create_generic(buffer, state);
                break;

            case library::capability::capability_type::PAGE_TABLE :
                kernel::utility::logger::printk("generic.convert -> generic\n");
                break;

            case library::capability::capability_type::FRAME :
                kernel::utility::logger::printk("generic.convert -> generic\n");
                break;

            default :
                kernel::utility::logger::printk("illegal type\n");
                break;
        }
        return 0;
    }

    common::error generic::create_generic(
        message_buffer *buffer,
        capability_local_state *state
    ) {};

    common::error generic::revoke()
    {
        return 0;
    }

}
