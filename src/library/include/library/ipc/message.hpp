#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <library/common/types.hpp>
#include <stdint.h>

namespace library::ipc
{
    struct message
    {
        common::sword sender_process_id;
        common::sword type;
        uint8_t data[1024];
    };
}

namespace kernel
{
    namespace ipc = library::ipc;
}

#endif
