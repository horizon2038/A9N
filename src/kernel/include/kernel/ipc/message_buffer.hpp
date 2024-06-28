#ifndef MESSAGE_BUFFER_HPP
#define MESSAGE_BUFFER_HPP

#include <library/common/types.hpp>
#include <library/common/array.hpp>

namespace a9n::kernel
{
    constexpr static a9n::word MESSAGE_BUFFER_MAX = 64;
    using message_buffer
        = liba9n::common::bounded_array<a9n::word, MESSAGE_BUFFER_MAX>;
}

#endif
