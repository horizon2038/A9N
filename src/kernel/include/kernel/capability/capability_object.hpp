#ifndef CAPABILITY_OBJECT_HPP
#define CAPABILITY_OBJECT_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <kernel/types.hpp>
#include <kernel/capability/capability_local_state.hpp>

namespace a9n::kernel
{
    class capability_object
    {
      public:
        virtual ~capability_object() {};

        virtual a9n::error
            execute(message_buffer *buffer, capability_local_state *local_state);

        virtual a9n::error remove();

        virtual a9n::error revoke(capability_local_state *local_state);
    };
}

#endif
