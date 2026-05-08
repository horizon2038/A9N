#ifndef CAPABILITY_LOCAL_STATE_HPP
#define CAPABILITY_LOCAL_STATE_HPP

#include <kernel/types.hpp>
#include <liba9n/libcxx/array>

namespace a9n::kernel
{
    inline constexpr a9n::word ENTRY_DATA_MAX = 3;

    using capability_slot_data                = liba9n::std::array<a9n::word, ENTRY_DATA_MAX>;

}

#endif
