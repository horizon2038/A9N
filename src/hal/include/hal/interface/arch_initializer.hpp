#ifndef HAL_ARCH_INITIALIZER_HPP
#define HAL_ARCH_INITIALIZER_HPP

#include <library/common/types.hpp>

namespace hal::interface
{
    class arch_initializer
    {
      public:
        virtual void init_architecture(common::word arch_info[]) = 0;
    };
}

#endif
