#ifndef CAPABILITY_OBJECT_HPP
#define CAPABILITY_OBJECT_HPP

namespace kernel
{
    class capability_object
    {
      public:
        virtual ~capability_object() {};

        virtual common::error
            execute(message_buffer *buffer, entry_local_state local_state);

        virtual common::error revoke();
    };
}

#endif
