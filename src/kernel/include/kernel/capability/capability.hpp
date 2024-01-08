#ifndef CAPABILITY_HPP
#define CAPABILITY_HPP

#include <library/common/types.hpp>
#include <stdint.h>

namespace kernel
{
    // currently, capabilities are provided in a hardware-independent manner.
    // however, to make extensions in the hardware-dependent part easier,
    // they take even values, following seL4.
    // [unused].
    enum class capability_type : uint8_t
    {
        UNINITIALIZED = 0,
        GENERIC = 2,
        CAPABILITY_NODE = 4,
        PROCESS_CONTROL_BLOCK = 6,
        IPC_OBJECT = 8,
        FRAME = 10
    };

    // capability_data is an essential presence for capability.execute().
    // This enables providing a common interface for capabilities such as
    // generic, frame, and others where we do not want to have a physical entity
    // in memory for each. This enhances extensibility and maintainability.
    struct capability_data
    {
        common::word elements[4];
    };

    class capability
    {
      public:
        virtual capability_type type() = 0;
        virtual common::error execute(capability_data data) = 0;
    };

}

#endif
