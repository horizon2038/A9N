#ifndef CAPABILITY_FACTORY_HPP
#define CAPABILITY_FACTORY_HPP

#include <kernel/capability/capability_component.hpp>
#include <library/common/types.hpp>

namespace kernel
{
    class capability_factory
    {
      public:
        common::word
            calculate_memory_size(common::word type, common::word size_flags);

        // make ??????
        capability_slot make(
            common::word type,
            common::word size_flags,
            common::virtual_address target_address
        );
    };
}

#endif
