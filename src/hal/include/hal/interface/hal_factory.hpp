#ifndef HAL_FACTORY_HPP
#define HAL_FACTORY_HPP

#include <interface/hal.hpp>

namespace hal::interface
{
    class hal_factory
    {
      public:
        virtual hal *make() = 0;
    };
}
#endif
