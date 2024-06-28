#ifndef CAPABILITY_FACTORY_HPP
#define CAPABILITY_FACTORY_HPP

#include <kernel/capability/capability_component.hpp>
#include <liba9n/common/types.hpp>
#include <liba9n/capability/capability_types.hpp>

namespace a9n::kernel
{
    class capability_factory
    {
      public:
        a9n::word calculate_memory_size_bits(
            liba9n::capability::capability_type type,
            a9n::word                           size_bits
        );

        // make ??????
        capability_slot make(
            liba9n::capability::capability_type type,
            a9n::word                           size_bits,
            a9n::virtual_address                target_address
        );
    };
}

#endif
