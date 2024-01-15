#ifndef MESSAGE_BUFFER_HPP
#define MESSAGE_BUFFER_HPP

#include <library/common/types.hpp>
#include <library/common/array.hpp>

namespace kernel
{
    constexpr static common::word MESSAGE_BUFFER_MAX = 8;
    using message_buffer
        = library::common::bounded_array<common::word, MESSAGE_BUFFER_MAX>;
}

#endif
