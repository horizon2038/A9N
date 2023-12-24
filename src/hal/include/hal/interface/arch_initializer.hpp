#ifndef HAL_ARCH_INITIALIZER_HPP
#define HAL_ARCH_INITIALIZER_HPP

namespace hal::interface
{
    class arch_initializer
    {
      public:
        virtual void init_architecture() = 0;
    };
}

#endif
