#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <liba9n/common/types.hpp>
#include <stdint.h>

namespace liba9n::ipc
{
    struct message
    {
        a9n::sword sender_process_id;
        a9n::sword type;
        uint8_t       data[1024];
    };
}

namespace a9n::kernel
{
    namespace ipc = liba9n::ipc;
}

#endif
