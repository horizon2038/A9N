#ifndef X86_64_HAL_FACTORY_HPP
#define X86_64_HAL_FACTORY_HPP

#include "../include/hal_factory.hpp"

namespace hal::x86_64
{
    class hal_factory : hal::hal_factory
    {
        public:
            hal make() override;
    };
}

#endif
