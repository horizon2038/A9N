#ifndef HAL_ARCH_INITIALIZER_HPP
#define HAL_ARCH_INITIALIZER_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

namespace a9n::hal
{
    class arch_initializer
    {
      public:
        // HAL-level initialization
        virtual hal_result init_architecture(a9n::word arch_info[]) = 0;
    };

    // TODO: devirtualization
    hal_result init_architecture();
}

#endif
