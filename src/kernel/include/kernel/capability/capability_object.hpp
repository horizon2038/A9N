#ifndef CAPABILITY_OBJECT_HPP
#define CAPABILITY_OBJECT_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <library/common/types.hpp>
#include <kernel/capability/capability_local_state.hpp>

namespace kernel
{
    class capability_object
    {
      public:
        virtual ~capability_object() {};

        virtual common::error
            execute(message_buffer *buffer, capability_local_state *local_state);

        virtual common::error remove();

        virtual common::error revoke(capability_local_state *local_state);
    };
}

#endif
