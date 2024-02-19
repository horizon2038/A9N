#ifndef CAPABILITY_FACTORY
#define CAPABILITY_FACTORY

#include <kernel/capability/capability_component.hpp>

#include <library/common/types.hpp>
#include <library/capability/capability_types.hpp>

namespace kernel
{
    class capability_factory
    {
      public:
        common::word calculate_real_size(
            library::capability::capability_type type,
            common::word size
        );

        capability_component *make(
            library::capability::capability_type type,
            common::virtual_address address
        );

      private:
        common::word calculate_capability_node_size(common::word size);
    };
}

#endif
