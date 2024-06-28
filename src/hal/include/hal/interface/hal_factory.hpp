#ifndef HAL_FACTORY_HPP
#define HAL_FACTORY_HPP

#include <hal/interface/hal.hpp>

namespace a9n::hal
{
    class hal_factory
    {
      public:
        virtual hal *make() = 0;
    };
}
#endif
