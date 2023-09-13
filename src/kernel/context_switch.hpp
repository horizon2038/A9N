#ifndef CONTEXT_SWITCH_HPP
#define CONTEXT_SWITCH_HPP

#include <interface/arch_context_switch.hpp>
#include "process.hpp"

namespace kernel
{
    class context_switch
    {
        public:
            context_switch();
            ~context_switch();
    };
}

#endif
