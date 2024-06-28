#ifndef CAPABILITY_DESCRIPTOR_HPP
#define CAPABILITY_DESCRIPTOR_HPP

#include <library/common/types.hpp>

namespace library::capability
{
    // capability_descriptors do not directly point to entries.
    // they are only used for indirect adressing.
    using capability_descriptor = common::word;

    struct capability_location
    {
        library::capability::capability_descriptor descriptor;
        common::word                               depth;
    };

}

#endif
