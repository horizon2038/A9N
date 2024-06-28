#ifndef CAPABILITY_FACTORY_HPP
#define CAPABILITY_FACTORY_HPP

#include <kernel/capability/capability_component.hpp>
#include <library/common/types.hpp>
#include <library/capability/capability_types.hpp>

namespace kernel
{
    class capability_factory
    {
      public:
        common::word calculate_memory_size_bits(
            library::capability::capability_type type,
            common::word                         size_bits
        );

        // make ??????
        capability_slot make(
            library::capability::capability_type type,
            common::word                         size_bits,
            common::virtual_address              target_address
        );
    };
}

#endif
