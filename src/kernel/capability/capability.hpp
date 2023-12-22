#ifndef CAPABILITY_HPP
#define CAPABILITY_HPP

#include <common.hpp>
#include <stdint.h>

namespace kernel
{
    // currently, capabilities are provided in a hardware-independent manner.
    // however, to make extensions in the hardware-dependent part easier,
    // they take even values, following seL4.
    enum class capability_type : uint8_t
    {
        UNINITIALIZED = 0,
        GENERIC = 2,
        CAPABILITY_NODE = 4,
        PROCESS_CONTROL_BLOCK = 6,
        FRAME = 8
    };

    struct capability
    {
        word_t data[4];
    };

}

#endif
