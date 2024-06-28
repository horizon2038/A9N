#ifndef CAPABILITY_DESCRIPTOR_HPP
#define CAPABILITY_DESCRIPTOR_HPP

#include <kernel/types.hpp>

namespace liba9n::capability
{
    // capability_descriptors do not directly point to entries.
    // they are only used for indirect adressing.
    using capability_descriptor = a9n::word;

    struct capability_location
    {
        liba9n::capability::capability_descriptor descriptor;
        a9n::word                                 depth;
    };

}

#endif
