#ifndef CAPABILITY_SLOT_HPP
#define CAPABILITY_SLOT_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_local_state.hpp>

namespace kernel
{
    struct capability_slot
    {
        capability_component *component;
        capability_local_state *state;
    };
}

#endif
