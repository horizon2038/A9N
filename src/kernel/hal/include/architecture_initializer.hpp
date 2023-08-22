#ifndef ARCHITECTURE_INITIALIZER_HPP
#define ARCHITECTURE_INITIALIZER_HPP

namespace hal
{
    class architecture_initializer
    {
        public:
            virtual void init_architecture() = 0;
    };
}

#endif
