#ifndef MESSAGE_BUFFER_HPP
#define MESSAGE_BUFFER_HPP

#include <library/common/types.hpp>

namespace kernel
{
    constexpr static common::word BUFFER_MAX = 8;

    struct message_buffer
    {
        common::word elements[BUFFER_MAX];
    };
}

#endif
