#ifndef HAL_ARCH_INITIALIZER_HPP
#define HAL_ARCH_INITIALIZER_HPP

#include <library/common/types.hpp>

namespace a9n::hal
{
    class arch_initializer
    {
      public:
        virtual void init_architecture(a9n::word arch_info[]) = 0;
    };
}

#endif
