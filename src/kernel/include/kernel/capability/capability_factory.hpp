#ifndef CAPABILITY_FACTORY_HPP
#define CAPABILITY_FACTORY_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    class capability_factory
    {
      public:
        a9n::word calculate_memory_size_bits(capability_type type, a9n::word size_bits);

        capability_slot
            make(capability_type type, a9n::word size_bits, a9n::virtual_address target_address);
    };
}

#endif
